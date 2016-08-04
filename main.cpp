#include <iostream>
#include <ctime>

#include "Document.h"
#include "ReadCorpus.h"
#include "DocumentOperation.h"
#include "GlossaryDao.h"



//静态变量的初始化
std::set<std::string> ReadCorpus::set_StopTerm;
std::vector<Sememe> ReadCorpus::vec_Sememe;

void DisplayDiffer(std::string str_SearchTxt,std::string str_OriginTxt)
{
    Document* docOrigin = new Document(str_OriginTxt,true);
    Document* docSearch = new Document(str_SearchTxt,true);
    if(docSearch->GetstrMD5() != docOrigin->GetstrMD5())
    {
        NLPIRUtil* nlpirUtil = new NLPIRUtil();
        nlpirUtil->SplitDocument(docOrigin);
        nlpirUtil->SplitDocument(docSearch);
        docOrigin->BuildInvertedIndex();
        //docOrigin->Display();
        delete nlpirUtil;
        DocumentOperation::GetDiffer(docOrigin,docSearch);

        //output docorigin
        docSearch->DisplayDiffer();
        docOrigin->DisplayDiffer();
        //docSearch->Display();

    }
    else
    {
        std::cout<<"two file similar!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<std::endl;
    }
    delete docOrigin;
    delete docSearch;
}

int main()
{
    //设置全局环境本地化
    std::locale::global(std::locale(""));
    struct timeval start,finish;
    double duration;
    gettimeofday(&start,NULL);

    /*//将词语义项存入数据库中
    GlossaryDao* glossaryDao = new GlossaryDao();
    std::string str_GlossaryPath="./dat/glossary.dat";
    glossaryDao->ReadGlossaryToDB(str_GlossaryPath);
    delete glossaryDao;*/

    /*//将词语逆文档频率存入数据库中
    IDFDao* idfDao = new IDFDao();
    std::string str_IDFPath="./Corpus/idf.txt";
    idfDao->ReadIDFToDB(str_IDFPath);
    delete idfDao;*/

    //读取语料库中的词频信息
    ReadCorpus::ReadStopTerm("./Corpus/StopTerm.txt");
    ReadCorpus::ReadSememe("./dat/whole.dat");

    //两个文本数据比较
    std::string str_Txt1 = "./test/search.txt";
    std::string str_Txt2 = "./test/origin.txt";
    DisplayDiffer(str_Txt1,str_Txt2);

    gettimeofday(&finish,NULL);
    duration = 1000000 * (finish.tv_sec - start.tv_sec) + finish.tv_usec - start.tv_usec;
    duration /= 1000000;
    std::cout<<std::endl<<std::endl<<"cost "<<duration<<" s"<<std::endl<<std::endl;
    return 0;
}
