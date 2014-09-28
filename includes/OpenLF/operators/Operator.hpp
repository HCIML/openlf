/*
* Copyright (c) 2014 Sven Wanner
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of 
* this software and associated documentation files (the "Software"), to deal in 
* the Software without restriction, including without limitation the rights to use,
* copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the 
* Software, and to permit persons to whom the Software is furnished to do so, 
* subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all 
* copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
* INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
* PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION 
* OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef OPERATOR_HPP
#define	OPERATOR_HPP


#include "OpenLF/debug.hpp"
#include "OpenLF/global.hpp"
#include "OpenLF/lightfield/Lightfield.hpp"


namespace OpenLF {
    namespace operators {
     
    

class Operator {
public:
    Operator(std::vector<std::string> inslots, std::vector<std::string> outslots);
    //Operator(const Operator& orig);
    //virtual ~Operator();
    void clear();
    
    virtual void process(lightfield::Lightfield & lf) = 0;
        
protected:
    
    // allocate memory for temporary storage
    virtual void allocate() = 0;
    // handle tasks before compute(), allocate output channel, etc.
    virtual void precompute(lightfield::Lightfield &lf) = 0;
    // core computational function that will be applied at EPI level
    virtual void compute(lightfield::Lightfield &lf) = 0;
    // handle tasks post EPI level compute function
    virtual void postcompute(lightfield::Lightfield &lf) = 0;
    // general clean up, i.e., temporary memory allocation
    virtual void cleanup() = 0;
    
    std::map<std::string,image::ImageChannel> tmp_memory;
    std::vector<std::string> inslots;
    std::vector<std::string> outslots;
};

}}
#endif	/* OPERATOR_HPP */

