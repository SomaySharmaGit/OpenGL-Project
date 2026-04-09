

#include<string>
#include<fstream>
#include<sstream>
#include<iostream>


std::string readFile(std::string fileName){

    std::ifstream file;
    std::string code;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        file.open(fileName);
       
        std::stringstream stream;
        stream << file.rdbuf();
        
        code = stream.str();
    }
    catch(std::ifstream::failure e)
    {
        std::cout << "ERROR:FILE_NOT_SUCCESSFULY_READ" << std::endl;
    }
    return code;


}