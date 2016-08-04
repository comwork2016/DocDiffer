#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include "NLPIR.h"

#include "../DataStructure.h"
#include "../corpus/ReadCorpus.h"
#include "../util/StringUtil.h"
#include "../util/HashUtil.h"
#include "WordIndex.h"
#include "../util/MD5.h"

// to delete
#include <iostream>

class Document
{
    public:
        Document(const std::string& str_DocPath,bool b_SplitToSentence = false);
        virtual ~Document();

        DOC_ID GetDocID() const { return m_DocID; }
        std::string GetstrDocPath() const { return m_strDocPath; }
        std::string GetstrDocName() const { return m_strDocName; }
        std::string GetstrContents() const { return m_strContents; }
        std::string GetstrMD5() const { return m_strMD5; }

        std::vector<Paragraph> GetvecParagraph() const { return m_vecParagraph; }
        void SetvecParagraph(std::vector<Paragraph> vec_Paragraph ) { m_vecParagraph = vec_Paragraph; }

        std::vector<TextRange> GetvecTextRange() const { return m_vecTextRanges; }
        void SetvecTextRange(std::vector<TextRange> vec_TextRange ) { m_vecTextRanges = vec_TextRange; }

        int GetnWordCount() { return m_nWordCount; }
        void SetnWordCount(int n_WordCount) { m_nWordCount = n_WordCount; }

        std::map<std::string, WordIndex*> GetMapWordIndex() const { return m_mapWordIndex; }

        bool GetSentenceByOffset(int offset, Sentence& sentence);

        void CalcDocSimHash();
        //void SplitSentencesToKGrams();
        void BuildInvertedIndex();
        void DisplayDiffer() const;
        void Display() const;

    protected:
        int ReadDocumentContent();
        int ReadDocumentAndSplitToSentence();
        void SplitParaphToSentence(Paragraph& para);
    private:

        DOC_ID m_DocID;
        std::string m_strDocPath;
        std::string m_strDocName;
        std::string m_strContents;

        std::string m_strMD5;

        std::vector<Paragraph> m_vecParagraph;
        int m_nWordCount; //文章中的有效词的总数
        std::map<std::string, double> m_mapTFIDF;//文档词频信息

        //当前文档中单词的倒排索引
        std::map<std::string,WordIndex*> m_mapWordIndex;

        //保存某一位置的文本是否相同
        std::vector<TextRange> m_vecTextRanges;
};

#endif // DOCUMENT_H
