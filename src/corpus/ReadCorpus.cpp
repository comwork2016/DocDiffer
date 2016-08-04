#include "ReadCorpus.h"

ReadCorpus::ReadCorpus()
{
    //ctor
}

/**
    读取停用词
*/
void ReadCorpus::ReadStopTerm(const std::string& str_Path)
{
    std::cout<<"Reading stopWords!"<<std::endl;
    std::ifstream fin;
    fin.open(str_Path.c_str(),std::ios::in);
    if(!fin.is_open())
    {
        std::cout<<"Read file: "<<str_Path<<" error!"<<std::endl;
        exit(ERROR_OPENFILE);
    }
    std::string line;
    while (getline(fin, line))
    {
        set_StopTerm.insert(line);
    }
    fin.close();
    std::cout<<"Reading stop words done!"<<std::endl;
}

/**
    把基本义原从文件读入vector
*/
void ReadCorpus::ReadSememe(const std::string str_filename)
{
    std::ifstream ifs(str_filename.c_str());
    if(!ifs.is_open())
    {
        std::cout<<"Read file: "<<str_filename<<" error!"<<std::endl;
        exit(ERROR_OPENFILE);
    }
    std::string str_line;
    while(getline(ifs,str_line))
    {
        std::istringstream iss(str_line);
        int index,pind;
        std::string str_sememe;
        iss>>index>>str_sememe>>pind;
        Sememe sem(index,str_sememe,pind);
        vec_Sememe.push_back(sem);
    }
    ifs.close();
}

ReadCorpus::~ReadCorpus()
{
    //dtor
}
