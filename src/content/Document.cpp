#include "Document.h"

Document::Document(const std::string& str_DocPath,bool b_SplitToSentence)
{
    //ctor
    this->m_DocID = str_DocPath;
    this->m_strDocPath = str_DocPath;
    int n_SeparatorIndex = str_DocPath.find_last_of("/");
    this->m_strDocName = str_DocPath.substr(n_SeparatorIndex+1);
    this->m_strContents = "";
    this->m_nWordCount = 0;
    std::cout<<"Reading document "<<this->m_strDocName<<std::endl;
    if(!b_SplitToSentence)
    {
        int n_ReadStats = ReadDocumentContent();
        if(n_ReadStats == ERROR_OPENFILE)
        {
            std::cout<<"read file "<<this->m_strDocPath<<" error"<<std::endl;
            exit(ERROR_OPENFILE);
        }
    }
    else
    {
        //读取文档内容
        int n_ReadStats = ReadDocumentAndSplitToSentence();
        if(n_ReadStats == ERROR_OPENFILE)
        {
            std::cout<<"read file "<<this->m_strDocPath<<" error"<<std::endl;
            exit(ERROR_OPENFILE);
        }
    }
    MD5 md5 = MD5(this->m_strContents);
    this->m_strMD5 = md5.hexdigest();
}

/**
    一次性读取全部的文档内容
*/
int Document::ReadDocumentContent()
{
    std::ifstream ifs_Doc;
    ifs_Doc.open((char *)this->m_strDocPath.c_str(),std::ios_base::in);
    if(!ifs_Doc.is_open())
    {
        return ERROR_OPENFILE;
    }
    std::stringstream ss;
    ss<<ifs_Doc.rdbuf();
    this->m_strContents = ss.str();
    ifs_Doc.close();
    return OK_READFILE;
}

/**
    读取文件内容
    程序中，将一行内容作为一个段落
    并用常见符号将段落分割成句子
*/
int Document::ReadDocumentAndSplitToSentence()
{
    std::ifstream ifs_Doc;
    ifs_Doc.open((char *)this->m_strDocPath.c_str(),std::ios_base::in);
    if(!ifs_Doc.is_open())
    {
        return ERROR_OPENFILE;
    }
    int offset=0;
    while(!ifs_Doc.eof())
    {
        Paragraph para;
        para.textRange.offset = offset;
        //读入一行数据
        std::string str_Line;
        std::getline(ifs_Doc,str_Line);
        int n_LineLength = str_Line.length();
        para.textRange.length = n_LineLength;
        this->m_strContents.append(str_Line);
        offset+=n_LineLength;
        // 不是文章结尾时将原文档的换行符加回去
        if(!ifs_Doc.eof())
        {
            this->m_strContents.append(1,'\n');
            offset++;
        }
        if(n_LineLength != 0 && !StringUtil::isStringBlank(str_Line)) //空白行不计为段落
        {
            //对段落进行句子的拆分
            SplitParaphToSentence(para);
            this->m_vecParagraph.push_back(para);
        }
    }
    ifs_Doc.close();
    return OK_READFILE;
}

/**
    将段落分割成句子
*/
void Document::SplitParaphToSentence(Paragraph& para)
{
    std::string str = this->m_strContents.substr(para.textRange.offset,para.textRange.length);
    //将文档用标点符号拆分
    std::vector<std::string> vec_pattern;
    vec_pattern.push_back("。");
    vec_pattern.push_back("？");
    vec_pattern.push_back("！");
    vec_pattern.push_back("；");
    const int strsize=str.size();
    std::string::size_type pos;
    int i = 0;
    while(i<strsize)
    {
        pos = strsize;
        int patternSize=0;
        //查找第一个分隔符的位置
        for(int j=0; j<vec_pattern.size(); j++)
        {
            std::string pattern = vec_pattern[j];
            int index =str.find(pattern,i);
            if(index!=std::string::npos && index < pos)
            {
                patternSize = pattern.size();
                pos = index+patternSize;
            }
        }
        if(pos<=strsize)
        {
            std::string s=str.substr(i,pos-i);
            if(!StringUtil::isStringBlank(s))
            {
                Sentence sen;
                sen.textRange.offset = para.textRange.offset + i;
                sen.textRange.length = pos-i;
                sen.firstWordNo = 0;
                para.vec_Sentences.push_back(sen);
            }
            i=pos;
        }
    }
}

/**
    构造倒排索引
*/
void Document::BuildInvertedIndex()
{
    for(int i=0; i<this->m_vecParagraph.size(); i++)
    {
        Paragraph para = this->m_vecParagraph[i];
        for(int j=0; j<para.vec_Sentences.size(); j++)
        {
            Sentence sen = para.vec_Sentences[j];
            for(int k=0; k<sen.vec_splitedHits.size(); k++)
            {
                SplitedHits sh = sen.vec_splitedHits[k];
                WordIndex* wordsIndex;
                if(this->m_mapWordIndex.find(sh.word)==this->m_mapWordIndex.end())//文档中第一次出现该单词
                {
                    wordsIndex = new WordIndex(sh.word,sh.textRange.length,sh.POS);
                }
                else
                {
                    wordsIndex = this->m_mapWordIndex[sh.word];
                }
                WordPos wordPos = {sh.textRange.offset,sh.NoInDoc,sen.textRange.offset};
                wordsIndex->AddDocPosInfo(this->m_strDocPath,wordPos);
                this->m_mapWordIndex[sh.word] = wordsIndex;
            }
        }
    }
}

/**
    获取句子范围长度
*/
bool Document::GetSentenceByOffset(int offset, Sentence& sentence)
{
    if(offset<0)
    {
        return -1;
    }
    for(int i=0; i<this->m_vecParagraph.size(); i++)
    {
        Paragraph para = m_vecParagraph[i];
        if(offset>para.textRange.offset+para.textRange.length)
        {
            continue;
        }
        //对句子进行分词并计算simhash
        for(int j = 0; j<para.vec_Sentences.size(); j++)
        {
            Sentence sen = para.vec_Sentences[j];
            if(sen.textRange.offset == offset)
            {
                sentence = sen;
                return true;
            }
            else if(sen.textRange.offset>offset)
            {
                return false;
            }
        }
    }
}

/**
    输出文件的信息
*/
void Document::Display() const
{
    std::cout<<this->m_strDocName<<std::endl;
    //输出段落句子的信息
    for(int i=0; i<this->m_vecParagraph.size(); i++)
    {
        Paragraph para = m_vecParagraph[i];
        //对句子进行分词并计算simhash
        for(int j = 0; j<para.vec_Sentences.size(); j++)
        {
            Sentence sen = para.vec_Sentences[j];
            std::string str_sentence = this->m_strContents.substr(sen.textRange.offset,sen.textRange.length);
            std::cout<<"Para "<<i<<" Sentence "<<j<<":["<<sen.textRange.offset<<","<<sen.textRange.length<<"]"<<std::endl;
            std::cout<<str_sentence<<std::endl<<std::endl;
            for(int k=0; k<sen.vec_splitedHits.size(); k++)
            {
                SplitedHits sh = sen.vec_splitedHits[k];
                std::cout<<sh.word<<"["<<sh.textRange.offset<<","<<sh.NoInDoc<<","<<sh.textRange.length<<","<<sh.POS<<"]"<<"\t";
                //std::cout<<sh.word<<"/"<<sh.pos<<"\t";
            }
            std::cout<<std::endl<<std::endl;
        }
    }
    /*查看倒排索引信息*/
    std::map<std::string,WordIndex*> map_WordIndex = this->m_mapWordIndex;
    for(std::map<std::string,WordIndex*>::iterator it = map_WordIndex.begin(); it != map_WordIndex.end(); it++)
    {
        WordIndex* wordIndex = it->second;
        wordIndex->Display();
        std::cout<<std::endl;
    }
    std::cout<<this->m_strMD5<<std::endl;
}

/**
    输出相似信息
*/
void Document::DisplayDiffer() const
{
    std::ofstream ofs;
    std::string str_DocName = this->m_strDocName.substr(0,this->m_strDocName.find("."));
    std::string str_outputFile = str_DocName+".html";
    ofs.open(str_outputFile.c_str(),std::ios::out);
    if(!ofs.is_open())
    {
        std::cout<<"Open file: "<<"  "<<" error!"<<std::endl;
        exit(ERROR_OPENFILE);
    }
    std::string str_Head = "<html xmlns=\"http://www.w3.org/1999/xhtml\"> \n" \
                           "<head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"><title>文本检测</title>\n" \
                           "<style type=\"text/css\">" \
                           "p{margin-bottom:10px;}" \
                           ".shubu{height: 20px;width: 20px;margin-left:25px;background-color: #FFFFFF;border: 1px solid #999999;text-align: center;vertical-align: middle;display: inline-block;color: #666666;}" \
                           "demo_padding {line-height: 30px;}" \
                           "a.red:link {color:#FF0000}a.red:visited {color:#FF0000}a.red:hover {color:#000000}a.red:active {color:#000000}" \
                           "a.orange:link {color:#FF6600}a.orange:visited {color:#FF6600}a.orange:hover {color:#000000}a.orange:active {color:#000000}" \
                           "a.dark:link {color:#666666}a.dark:visited {color:#666666}a.dark:hover {color:#000000}a.dark:active {color:#000000}" \
                           ".green{color:#008000}.gray{color:#666666}.red{color:#FF0000}.orange{color:#FF6600}" \
                           "</style>";
    ofs.write(str_Head.c_str(),str_Head.size());

    std::string str_CR = "\n";
/*    std::string str_Contents = this->m_strContents;
    StringUtil::ReplaceString(str_Contents,str_CR,"<br>");
    ofs.write(str_Contents.c_str(),str_Contents.size());
    std::string separator = "<br><br>------------------------------------------------<br><br>";
    ofs.write(separator.c_str(),separator.size());
*/
    int offset = 0;
    for(int i=0; i<this->m_vecTextRanges.size(); i++)
    {
        TextRange textRange = this->m_vecTextRanges[i];
        if(offset < textRange.offset)
        {
            std::string str_Text = "<span class=\"red\">" + this->m_strContents.substr(offset,textRange.offset - offset)+"</span>";
            StringUtil::ReplaceString(str_Text,str_CR,"<br>");
            ofs.write(str_Text.c_str(),str_Text.size());
            i--;
        }
        else
        {
            std::string str_Text = this->m_strContents.substr(textRange.offset,textRange.length);
            //std::cout<<textRange.n_Similar<<":"<<str_Text<<std::endl;
            //std::cin.get();
            std::string str_Sen;
            if(textRange.n_Similar == TextSame)
            {
                str_Sen = "<span class=\"green\">"+ str_Text +"</span>";
            }
            else if(textRange.n_Similar == TextSimilar)
            {
                str_Sen = "<span class=\"orange\">"+ str_Text +"</span>";
            }
            else if(textRange.n_Similar == TextDifferent)
            {
                str_Sen = "<span class=\"red\">"+ str_Text +"</span>";
            }
            else if(textRange.n_Similar == TextIgnore)
            {
                str_Sen = "<a class=\"gray\">"+ str_Text +"</a>";
            }
            ofs.write(str_Sen.c_str(),str_Sen.size());
        }
        offset = textRange.offset + textRange.length;
    }
    if(offset<this->m_strContents.size())
    {
        std::string str_Text = "<span class=\"red\">" + this->m_strContents.substr(offset,this->m_strContents.size())+"</span>";
        StringUtil::ReplaceString(str_Text,str_CR,"<br>");
        ofs.write(str_Text.c_str(),str_Text.size());
    }
    std::string str_End = "</body></html>";
    ofs.write(str_End.c_str(),str_End.size());
    ofs.close();
}

Document::~Document()
{
    //释放倒排索引所占的资源
    for(std::map<std::string,WordIndex*>::iterator it = this->m_mapWordIndex.begin(); it != this->m_mapWordIndex.end(); it++)
    {
        delete it->second;
    }
}
