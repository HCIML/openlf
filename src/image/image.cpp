#include "image/image.hpp"
#include "visualize/imgshow.hpp"

OpenLF::Image::Image() 
/* TEST in image_test::testConstructor() */
{
    print(1,"Image() called...");
    
    // set properties
    this->_width = 0;
    this->_height = 0;
    this->_label = "";
}

OpenLF::Image::Image(int width, int height, int channels)  
/* TEST in image_test::testConstructor() */
{
    print(1,"Image(int,int,int) called...");
    if(channels < 0) throw  OpenLF_Exception("Channel needs to be a posiiv integer in Constructor!");
    
    // set properties
    this->_width = width;
    this->_height = height;
    
    // specifiy label depending on number of channels
    switch(channels) {
        case 1: this->_label = "bw"; break;
        case 2: this->_label = "vec"; break;
        case 3: this->_label = "rgb"; break;
        default: this->_label = ""; break;
    }
    
    // allocate memory
    for(int c=0; c<channels; c++) 
        this->_data.push_back(new vigra::MultiArray<2,float>(vigra::Shape2(this->_width,this->_height)));               
}

OpenLF::Image::Image(const char* filename)  
/* TEST in image_test::testConstructor() */
{
    print(1,"Image(const char*) called..");
    load(filename);
}

OpenLF::Image::Image(string filename)  
/* TEST in image_test::testConstructor() */
{
    print(1,"Image(string) called..");
    load(filename.c_str());
}

OpenLF::Image::Image(const Image& orig) 
/* image_test::testCopyConstructor() */
{       
    print(1,"Image(const Image&) called..");
    
    // set properties
    this->_width = orig.width();
    this->_height = orig.height();
    this->_label = orig.label();
    
    // pointer to data
    float* data_ptr = NULL;
    float* other_data_ptr = NULL;
    
    // allocate memory
    for(int c=0; c<orig.channels(); c++) {
        this->_data.push_back(new vigra::MultiArray<2,float>(vigra::Shape2(this->_width,this->_height)));
        
        try {
            // get data pointer of current channel
            data_ptr = this->_data[c]->data();
            other_data_ptr = orig._data[c]->data();
            if(!data_ptr) throw OpenLF_Exception("Getting this data pointer failed in Image:: CopyConstructor!");
            if(!other_data_ptr) throw OpenLF_Exception("Getting other data pointer failed in Image:: CopyConstructor!");

            // copy data 
            for(int n=0; n<this->_width*this->_height; n++) {
                data_ptr[n] = other_data_ptr[n];
            }
        } catch(exception & e) {
            cout << e.what() << endl;
        }
        
    }
}

OpenLF::Image::~Image() 
{
    for(int c=0; c<channels(); c++) {
        this->_data[c]->~MultiArray();
        delete this->_data[c];
    }
    this->_data.clear();
}









/* ######################
   GETTER SETTER METHODS
   ###################### */
    
int OpenLF::Image::width() const  
/* TEST in image_test::testConstructor() */
{
    return this->_width;
}

int OpenLF::Image::height() const  
/* TEST in image_test::testConstructor() */
{
    return this->_height;
}

int OpenLF::Image::channels() const  
/* TEST in image_test::testConstructor() */
{
    return this->_data.size();
}

string OpenLF::Image::label() const  
/* TEST in image_test::testConstructor() */
{
    return this->_label;
}

void OpenLF::Image::set_label(string label)  
/* TEST in image_test::testConstructor() */
{
    this->_label = label;
}

void OpenLF::Image::fill_image_channel(int channel, float value, float std)  
/* TEST in image_test::testConstructor() */
{
    srand (time(NULL));
    float std_dev = 0;
    float tmp_value = 0; 
    float* data_ptr = NULL;
    
    try {
        // get data pointer 
        data_ptr = this->_data[channel]->data();
        
        if(this->_data.size()<=0 || channel >= this->_data.size() || data_ptr == NULL)
            throw OpenLF_Exception("Channel access exception in Image::fill_image_channel()!");

        // fill channel with value plus a random value if std>0
        for(int n=0; n<this->_width*this->_height; n++) {
            if(std>0) {
                std_dev = (float)rand()/RAND_MAX*std;
            }
            data_ptr[n] = value+std_dev;
        }
    } catch(exception & e) {
        cout << e.what() << endl;
    }
}

void OpenLF::Image::add_channel() {
/* TEST in image_test::addCannel() */
    if(this->_width>0 && this->_height>0) {
        this->_data.push_back(new vigra::MultiArray<2,float>(vigra::Shape2(this->_width,this->_height)));
        *this->_data[this->_data.size()-1] = 0;
    }
}

void OpenLF::Image::add_channel(float* data) {
/* TEST in image_test::addCannel() */
    float* data_ptr = NULL;
    try {
        this->_data.push_back(new vigra::MultiArray<2,float>(vigra::Shape2(this->_width,this->_height)));
        data_ptr = this->_data[this->_data.size()-1]->data();
        
        if(data_ptr == NULL)
            throw OpenLF_Exception("Channel access exception in Image::add_channel()!");
        
        // copy data
        for(int n=0; n<this->_width*this->_height; n++) {
            data_ptr[n] = data[n];
        }
    } catch(exception & e) {
        cout << e.what() << endl;
    }
}

void OpenLF::Image::add_channel(int channel, const OpenLF::Image& orig)
/* Test: image_test::conversionOpenCV() */
{ 
    try {
        // pointer to data arrays
        float* data_ptr = NULL;
        float* orig_data_ptr = NULL;
    
        // check for shape consistency
        if(orig.width()!=this->_width || orig.height() != this->_height)
            throw OpenLF_Exception("Shape mismatch in Image::add_channel(const OpenLF::Image&)!");
        
        // add MultiArray to _data and get data pointers
        this->_data.push_back(new vigra::MultiArray<2,float>(vigra::Shape2(this->_width,this->_height)));
        data_ptr = this->_data[this->_data.size()-1]->data();
        orig_data_ptr = orig.get_channel(channel);
        
        if(data_ptr == NULL)
            throw OpenLF_Exception("Channel access exception in Image::add_channel(int, const OpenLF::Image&)!");
        if(orig_data_ptr == NULL)
            throw OpenLF_Exception("Channel access in passed instance exception in Image::add_channel(int, const OpenLF::Image&)!");
        
        // copy data
        for(int n=0; n<this->_width*this->_height; n++) {
            data_ptr[n] = orig_data_ptr[n];
        }
    } catch(exception & e) {
        cout << e.what() << endl;
    }
}

void OpenLF::Image::set_channel(float* data, int channel) 
/* TEST in image_test::addCannel() */
{
    float* data_ptr = NULL;
    try {
        if(channel < 0 || channel >= this->_data.size())
            throw OpenLF_Exception("Access to not existing channel exception in Image::set_channel()!");
        
        /// get data pointer
        data_ptr = this->_data[channel]->data();
        if(data_ptr == NULL)
            throw OpenLF_Exception("Channel access exception in Image::add_channel()!");
        
        // copy data
        for(int n=0; n<this->_width*this->_height; n++) {
            data_ptr[n] = data[n];
        }
        
    } catch(exception & e) {
        cout << e.what() << endl;
    }
}

void OpenLF::Image::copy_channel(int channel, const Image& orig)
/* TEST in image_test::conversionOpenCV() */
{
    try {
        // delete existing data
        if(this->_data.size()>0) {
            for(int c=0; c<this->_data.size(); c++) {
                this->_data[c]->~MultiArray();
            }
            this->_data.clear();
        }
        
        float* orig_data = NULL;

        /// set properties
        this->_width = orig.width();
        this->_height = orig.height();
        this->_label = "bw";

        // copy channel
        this->_data.push_back(new vigra::MultiArray<2,float>(vigra::Shape2(this->_width,this->_height)));
        orig_data = orig.get_channel(channel);
        if(orig_data == NULL) OpenLF_Exception("Channel access exception in Image::copy_channel(int, const Image&)!");
        for(int n=0; n<this->_width*this->_height; n++)
            this->_data[0]->data()[n] = orig_data[n];
            
    } catch(exception & e) {
        cout << e.what() << endl;
    }
}




/* ######################
         IO METHODS
   ###################### */

void OpenLF::Image::load(const char* filename)  
/* TEST in image_test::testConstructor() */
{    
    //TODO: check if file is .exr and load without norming   
    
    //if image is not empty, delete channels
    if(this->_data.size()>0) {
        for(int c=0; c<channels(); c++) {
            this->_data[c]->~MultiArray();
        }
        this->_data.clear();
    }
    
 
    try {
        // import image info from file
        vigra::ImageImportInfo info(filename);
        
        if(info.isGrayscale()) {
            print(1,"load grayscale image...");
            
            // uint image to import data from file
            vigra::MultiArray<2, vigra::UInt8> in(info.width(), info.height());
            
            // allocate memory
            this->_data.push_back(new vigra::MultiArray<2,float>(info.width(), info.height()));
           
            // import data
            vigra::importImage(info, vigra::destImage(in));
            
            // copy data into object and map to range [1,0]
            vigra::UInt8* data_ptr = in.data();
            for(int n=0; n<info.width()*info.height(); n++) {
                this->_data[0]->data()[n] = ((float)data_ptr[n])/255.0;
            }
           
            // set properties
            this->_width = info.width();
            this->_height = info.height();
            this->_label = "bw";
        
        } else if(info.isColor()) {
            print(1,"load rgb image...");
            
            // uint rgb image to import data from file
            vigra::MultiArray<2, vigra::RGBValue<vigra::UInt8> > in(info.shape());
            
            // allocate memory
            for(int c=0; c<3; c++)
                this->_data.push_back(new vigra::MultiArray<2,float>(info.width(), info.height()));
       
            // import data
            vigra::importImage(info, in);
            
            // copy data into object and map to range [1,0]
            int index;
            for(int x=0; x<info.width(); x++) {
                for(int y=0; y<info.height(); y++) {
                    index = y*info.width()+x;
                    this->_data[0]->data()[index] = in(x,y)[0]/255.0f;
                    this->_data[1]->data()[index] = in(x,y)[1]/255.0f;
                    this->_data[2]->data()[index] = in(x,y)[2]/255.0f;
                }
            }
            
            // set properties
            this->_width = info.width();
            this->_height = info.height();
            this->_label = "rgb";
            
        } else {
            throw OpenLF_Exception("Only grayscale and rgb images supported in Image::load()!");
        }
    }
    catch(exception & e) {
        cout << e.what() << endl;
    }
}


void OpenLF::Image::save(string filename)  
/* TEST in image_test::testConstructor() */
{
    //TODO: add OpenEXR export
    
    string ftype = OpenLF::find_ftype(filename);
    
    if(this->_label == "bw") {
        string msg = "save bw image as " + ftype +"..."; print(1,msg.c_str());
        
        if(ftype=="jpg")
            vigra::exportImage(*this->_data[0], vigra::ImageExportInfo(filename.c_str()).setCompression("JPEG QUALITY=75"));
        else
            vigra::exportImage(*this->_data[0], filename.c_str());
    }
    
    else {
        string msg = "save " + this->_label + " image as " + ftype +"..."; print(1,msg.c_str());
     
        //make rgb container
        vigra::MultiArray<2, vigra::RGBValue<vigra::UInt8> > out_img(vigra::Shape2(this->_width,this->_height));
        out_img.init(0);
        
        //fill rgb container with data
        get_rgb(out_img);
    
        if(ftype=="jpg")
            vigra::exportImage(out_img, vigra::ImageExportInfo(filename.c_str()).setCompression("JPEG QUALITY=75"));
        else
            vigra::exportImage(out_img, filename.c_str());
    }

}







/* ######################
    ACCESS DATA METHODS
   ###################### */

float* OpenLF::Image::get_channel(int channel) const
/* TEST in image_test::testCannelAcccess() */
{
    print(3,"get_channel(int)");    
    if(channel>=0 && channel<this->_data.size()) {
        return this->_data[channel]->data();
    }
}

void OpenLF::Image::swap_channel(int channel, vigra::MultiArray<2,float>& array) 
/* TEST in image_test::testCannelAcccess() */
{
    print(3,"get_channel(int)");    
    if(channel<0 || channel>=this->_data.size()) throw OpenLF_Exception("channel doesn't exist exception in Image::swap_channel(int,MultiArray<2,float>&)!");
    try {
        // swap data of channel into array by copying shape and data pointer
        this->_data[channel]->swap(array);
    } catch(exception & e) {
        cout << e.what() << endl;
    }
}

float OpenLF::Image::access_pixel(int x,int y, int channel)
/* TEST in image_test::testCannelAcccess() */
{
    print(3,"access_pixel(int,int,int)");
    try {
        if(channel<0 || channel>=this->_data.size()) throw OpenLF_Exception("channel doesn't exist exception in Image::acccess_pixel(int,int,int)!");
        if(x<0 || x>=this->_width) throw OpenLF_Exception("out of range row access in Image::acccess_pixel(int,int,int)!");
        if(y<0 || y>=this->_height) throw OpenLF_Exception("out of range column access in Image::acccess_pixel(int,int,int)!");
        return this->_data[channel]->data()[x*this->_width+y];
    } catch(exception & e) {
        cout << e.what() << endl;
    }
}

void OpenLF::Image::get_pixel(int x, int y, vector<float> &values)  
/* TEST in image_test::testConstructor() */
{
    print(3,"get_pixel(int,int,vector<float>)");
    values.clear();
    for(int c=0; c<this->_data.size(); c++) {
        values.push_back(access_pixel(x,y,c));
    }
}

void OpenLF::Image::get_rgb(vigra::MultiArray<2, vigra::RGBValue<vigra::UInt8> >& rgb_img) 
/* TEST in image_test::testConstructor() */
{      
    // copy data into vigra RGB image
    int n=0;
    for(int x=0; x<this->_width; x++) {
        for(int y=0; y<this->_height; y++) {
            for(int c=0; c<3; c++) {
                if(c<this->_data.size())
                    rgb_img.data()[n][c] = (int)(this->_data[c]->data()[this->_width*x+y]*255);
                else
                    rgb_img.data()[n][c] = 0;
            }
            n++;
        }
    }
}

void OpenLF::Image::get_opencv(cv::Mat& cv_mat) 
/* TEST in image_test::conversionOpenCV() */
{
    try {
        if(label()=="bw") { 
            if(this->_data.size() > 0) { 
                cv_mat = cv::Mat::zeros(this->_height,this->_width,CV_32FC1);
                cv_mat.data = (uchar*)this->_data[0]->data();
            } else throw OpenLF_Exception("Unresolved bw channel access in Image::get_opencv(cv::Mat&)!");
        }
        else if(label()=="vec") {
            if(this->_data.size() > 1) { 
                cv_mat = cv::Mat::zeros(this->_height,this->_width,CV_32FC2);

                for(int x=0; x<width(); x++) {
                    for(int y=0; y<height(); y++) {
                        cv_mat.at<cv::Vec2f>(y,x)[0] = this->_data[0]->data()[width()*y+x];
                        cv_mat.at<cv::Vec2f>(y,x)[1] = this->_data[1]->data()[width()*y+x];
                    }
                }       
            } else throw OpenLF_Exception("Unresolved vector channel access in Image::get_opencv(cv::Mat&)!");
        }
        else if(label()=="rgb") {
            if(this->_data.size() > 1) {
                cv_mat = cv::Mat::zeros(this->_height,this->_width,CV_32FC3);

                for(int x=0; x<width(); x++) {
                    for(int y=0; y<height(); y++) {
                         cv_mat.at<cv::Vec3f>(y,x)[0] = this->_data[2]->data()[width()*y+x];
                         cv_mat.at<cv::Vec3f>(y,x)[1] = this->_data[1]->data()[width()*y+x];
                         cv_mat.at<cv::Vec3f>(y,x)[2] = this->_data[0]->data()[width()*y+x];
                    }
                } 
            } else throw OpenLF_Exception("Unresolved rgb channel access in Image::get_opencv(cv::Mat&)!");
        }
        
    } catch(exception & e) {
        cout << e.what() << endl;
    }
}

void OpenLF::Image::get_opencv(int channel, cv::Mat& cv_mat) 
/* TEST in image_test::conversionOpenCV() */
{
    try {
        if(this->_data.size() > 0 && channel < this->_data.size()) { 
            cv_mat = cv::Mat::zeros(this->_height,this->_width,CV_32FC1);
            cv_mat.data = (uchar*)this->_data[channel]->data();
        } else throw OpenLF_Exception("Unresolved channel access in Image::get_opencv(int,cv::Mat&)!");
        
    } catch(exception & e) {
        cout << e.what() << endl;
    }
}
