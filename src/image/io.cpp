#include "image/io.hpp"



void OpenLF::image::io::reduce_channels(map<string,vigra::MultiArray<2,float>> &channels,vector<string> keys_to_keep)
/* TEST: via imsave in test_image::test_io() */
{
    print(2,"image::io::reduce_channels(channels,keys_to_keep) called...");
    
    // if no labels to keep passed return void
    if(keys_to_keep.size() == 0) return;

    // loop through map and erase channels not in keys_to_keep
    for(map<string, vigra::MultiArray<2,float>>::iterator i = channels.begin(); i != channels.end(); ++i)
    {
        // get key
        string key = i->first;
        
        // check if key is in keys_to_keep
        int key_found = 0;
        for(int c=0; c<keys_to_keep.size(); c++) {
            if(key==keys_to_keep[c]) {
                key_found++;
            }
        }

        // if not found erase channel
        if(key_found==0) {
            cout << "erase " << key << endl;
            channels.erase(key);
        }
    }
}



void OpenLF::image::io::linear_range_mapping(vigra::MultiArray<2,float>& fimg, vigra::MultiArray<2, vigra::UInt8>& img) 
/* TEST: via imsave in test_image::test_io() */
{
    print(2,"image::io::linear_range_mapping(fimg,img) called...");
    
    try {
        // functor to find range
        vigra::FindMinMax<vigra::FImage::PixelType> minmax;
        
        // find original range
        vigra::inspectImage(vigra::srcImageRange(fimg), minmax);
        if(minmax.max<=minmax.min) throw OpenLF_Exception("image::io::linear_range_mapping failed, no distance in image to map!");
        
        
        int nmin, nmax;
        // if original range is btw [0,1] keep relative range by multiplying min/max with 255
        if(minmax.min>=0 && minmax.min<1 && minmax.max>0 && minmax.max<=1) {
            nmin = int(ceil(minmax.min*255));
            nmax = int(floor(minmax.max*255));
        }
        // else scale hard to [0,255]
        else {
            nmin = 0;
            nmax = 255;
        }
            
        // transform the range to min/max specified
        vigra::transformImage(vigra::srcImageRange(fimg), vigra::destImage(img),
                              vigra::linearRangeMapping( minmax.min, minmax.max, nmin, nmax) );
    } catch(exception & e) {
        cout << e.what() << endl;
    }
}




bool OpenLF::image::io::imread(string filename, map<string,vigra::MultiArray<2,float>> &channels)
/* TEST: test_image::test_io() */
{
    print(2,"image::io::imread(string,map) called...");
    
    try {
        // import image info to get the image shape
        vigra::ImageImportInfo info(filename.c_str());
    
        // image size
        int width = info.width();
        int height = info.height();

        // load grayscale images
        if(info.isGrayscale()) {
            if(info.numBands()!=1) throw OpenLF_Exception("Image is not a grayscale image, alpha channels not supported yet!");
            print(3,"image::io::imread(string,map) found grayscale image...");
            
            // create channel
            channels["bw"] = vigra::MultiArray<2,float>(vigra::Shape2(info.width(),info.height()));

            // uint image to import data from file
            vigra::MultiArray<2, vigra::UInt8> in(width,height);

            // import data
            vigra::importImage(info, vigra::destImage(in));

            // copy data into object and map to range [1,0]
            for(int y=0; y<height; y++) {
                for(int x=0; x<width; x++) {
                    channels["bw"](x,y) = ((float)in(x,y))/255.0;
                }
            }
        }
        // load color images
        else if(info.isColor()) {
            if(info.numBands()!=3) throw OpenLF_Exception("Image is not a rgb image, alpha channels not supported yet!");
            print(3,"image::io::imread(string,map) found color image...");

            // create channels
            channels["r"] = vigra::MultiArray<2,float>(vigra::Shape2(width,height)); 
            channels["g"] = vigra::MultiArray<2,float>(vigra::Shape2(width,height)); 
            channels["b"] = vigra::MultiArray<2,float>(vigra::Shape2(width,height)); 

            // uint rgb image to import data from file
            vigra::MultiArray<2, vigra::RGBValue<vigra::UInt8> > in(info.shape());

            // import data
            vigra::importImage(info, in);                   

            // copy data into lf container
            for(int y=0; y<height; y++) {
                for(int x=0; x<width; x++) {
                    channels["r"](x,y) = ((float)in(x,y)[0])/255.0f;
                    channels["g"](x,y) = ((float)in(x,y)[1])/255.0f;
                    channels["b"](x,y) = ((float)in(x,y)[2])/255.0f;
                }
            }
        }
    }
    catch(exception &e) {
        cout << e.what() << endl;
        return false;
    }
    return true;
}




bool OpenLF::image::io::imsave(string filename, vigra::MultiArray<2,float> img)
/* TEST: test_image::test_io() */
{
    print(2,"image::io::imread(filename,img) called...");
    
    // get file type
    string ftype = OpenLF::helpers::find_ftype(filename);
    
    try {
        // allocate memory to store range mapping results
        vigra::MultiArray<2,vigra::UInt8> tmp(vigra::Shape2(img.width(),img.height()));
        linear_range_mapping(img,tmp);

        if(ftype=="jpg")
            vigra::exportImage(tmp, vigra::ImageExportInfo(filename.c_str()).setCompression("JPEG QUALITY=75"));
        else
            vigra::exportImage(tmp, filename.c_str());
        
    } catch(exception & e) {
        cout << e.what() << endl;
        return false;
    }
    
    return true;
}





bool OpenLF::image::io::imsave(string filename, map<string,vigra::MultiArray<2,float>> &channels, string key)
/*TEST: test_image::test_io() */
{
    print(2,"image::io::save(filename,channels,key) called...");

    if ( channels.count(key) != 0 ) {
        return imsave(filename, channels[key]);
    }
}





bool OpenLF::image::io::imsave(string filename, map<string,vigra::MultiArray<2,float>> channels)
/* TEST: test_image::test_io() */
{
    print(2,"image::io::imread(string,map) called...");

    string ftype = OpenLF::helpers::find_ftype(filename);
    
    try {
        
        // if a grayscale channel exist save
        if ( channels.count("bw") != 0 ) {

            string msg = "save image as grayscale " + ftype +"..."; print(3,msg.c_str());

            // adapt filename to set the appendix
            string bw_filename = string(filename);
            int insert_pos = filename.length()-(ftype.length()+1);
            bw_filename.insert(insert_pos,"_bw");

            // save channel
            imsave(bw_filename,channels["bw"]);

            // delete bw channel
            channels.erase("bw");
        }
        
        // if r,g and b channels exist save
        if ( channels.count("r") != 0 && channels.count("g") != 0  && channels.count("b") != 0 ) {

            string msg = "save image as color " + ftype +"...";  print(3,msg.c_str());

            // adapt filename to set the appendix
            string rgb_filename = string(filename);
            int insert_pos = filename.length()-(ftype.length()+1);
            rgb_filename.insert(insert_pos,"_rgb");

            // allocate output container
            vigra::MultiArray<2, vigra::RGBValue<vigra::UInt8> > rgb(vigra::Shape2(channels["r"].width(),channels["r"].height()));

            // allocate memory to store range mapping results
            vigra::MultiArray<2,vigra::UInt8> tmp(vigra::Shape2(channels["r"].width(),channels["r"].height()));

            // range map data and copy to output image
            vector<string> channel_labels {"r","g","b"};
            for(int c=0; c<3; c++) {
                
                // map channel to [0,255]
                linear_range_mapping(channels[channel_labels[c]],tmp);

                // copy data into rgb container
                for(int y=0; y<channels[channel_labels[c]].height(); y++) {
                    for(int x=0; x<channels[channel_labels[c]].width(); x++) {
                        rgb(x,y)[c] = tmp(x,y);
                    }
                }

                // delete r,g and b channel
                channels.erase(channel_labels[c]);
            }

            if(ftype=="jpg")
                vigra::exportImage(rgb, vigra::ImageExportInfo(rgb_filename.c_str()).setCompression("JPEG QUALITY=75"));
            else
                vigra::exportImage(rgb, rgb_filename.c_str());        
        }

        // check for other channels and save them
        for(map<string, vigra::MultiArray<2,float>>::iterator i = channels.begin(); i != channels.end(); ++i)
        {
            // get key
            string key = i->first;

            // adapt filename to set the appendix
            string tmp_filename = string(filename);
            int insert_pos = filename.length()-(ftype.length()+1);

            string appendix = "_"+key;
            tmp_filename.insert(insert_pos,appendix);

            // save and delete channel
            imsave(tmp_filename,channels[key]);
            channels.erase(key);
        }
        
    } catch(exception & e) {
        cout << e.what() << endl;
        return false;
    }
    
    return true;
}



bool OpenLF::image::io::imsave(string filename, map<string,vigra::MultiArray<2,float>> channels, vector<string> keys_to_save)
/* TEST: test_image::test_io() */
{
    print(2,"image::io::imsave(filename, channels, keys_to_save) called...");
    
    try {
        // delete unnecessary channels and save
        reduce_channels(channels,keys_to_save);
        return imsave(filename,channels);
    }
    catch(exception & e) {
        cout << e.what() << endl;
        return false;
    }
}