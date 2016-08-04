#include "NLPIRUtil.h"

NLPIRUtil::NLPIRUtil()
{
    //初始化分词组件
    if(!NLPIR_Init(0,UTF8_CODE,0))
    {
        std::cout<<"ICTCLAS INIT FAILED!"<<std::endl;
    }
    NLPIR_ImportUserDict("./Corpus/userdic.txt");
}

std::vector<SplitedHits> NLPIRUtil::SplitStringToWords(std::string str,int n_FirstWordNo,int n_SentencePos)
{
    std::vector<SplitedHits> vec_Result;
    const result_t *pVecResult;
    int nCount = 0;
    pVecResult = NLPIR_ParagraphProcessA(str.c_str(),&nCount,true);
    for (int i=0; i<nCount; i++)
    {
        std::string str_HitsWord = str.substr(pVecResult[i].start,pVecResult[i].length);
        if(StringUtil::isStringBlank(str_HitsWord) || ReadCorpus::set_StopTerm.find(str_HitsWord)!=ReadCorpus::set_StopTerm.end())
        {
            continue;
        }
        //筛选词性名词(N)、动词(V)、形容词 (A)、数词(M)、量词(Q)和时间(T)
        char ch_pos = pVecResult[i].sPOS[0];
        if(ch_pos == 'n' || ch_pos == 'v' || ch_pos == 'n' || ch_pos == 'a' || ch_pos == 'm' || ch_pos == 'q' || ch_pos == 't')
        {
            //SIMHASH_TYPE simhash = HashUtil::CalcStringHash(str_HitsWord);
            TextRange textRange =
            {
                n_SentencePos + pVecResult[i].start,//偏移值为最后一个字符的下一个字符所在的位置
                pVecResult[i].length
            };
            SplitedHits sh_hits =
            {
                str_HitsWord,
                textRange,
                pVecResult[i].sPOS,
                n_FirstWordNo
            };
            vec_Result.push_back(sh_hits);
            n_FirstWordNo++;
        }
    }
    return vec_Result;
}

/**
    将文档中的句子分词
*/
void NLPIRUtil::SplitDocument(Document* doc)
{
    std::string str_Contents = doc->GetstrContents();
    std::vector<Paragraph> vec_Paragraph;
    for(int i=0; i<doc->GetvecParagraph().size(); i++)
    {
        Paragraph para = doc->GetvecParagraph()[i];
        //对句子进行分词处理
        for(int j = 0; j<para.vec_Sentences.size(); j++)
        {
            Sentence& sen = para.vec_Sentences[j];
            std::string str = str_Contents.substr(sen.textRange.offset,sen.textRange.length);
            int n_WordCount = doc->GetnWordCount();
            sen.vec_splitedHits = SplitStringToWords(str,n_WordCount,sen.textRange.offset);
            doc->SetnWordCount(n_WordCount + sen.vec_splitedHits.size());
        }
        vec_Paragraph.push_back(para);
    }
    doc->SetvecParagraph(vec_Paragraph);
}

NLPIRUtil::~NLPIRUtil()
{
    NLPIR_Exit();
}
