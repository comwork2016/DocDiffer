#include "DocumentOperation.h"

DocumentOperation::DocumentOperation()
{
    //ctor
}

/**
    以句子为单位，查询所有可能的句子中相同词语的个数
*/
std::map<int,int> GetSentenceOccurTimes(Sentence sen,Document* docOrigin,std::map<std::string,WordIndex*> map_DocOriginMapWordIndex)
{
    std::map<int,int> map_SentenceOccurTimes;
    std::set<std::string> set_Words;//一个句子中的词语只算作一次
    for(int k=0; k<sen.vec_splitedHits.size(); k++)
    {
        std::string str_Word = sen.vec_splitedHits[k].word;
        if(set_Words.find(str_Word) != set_Words.end())
        {
            continue;
        }
        set_Words.insert(str_Word);
        if(map_DocOriginMapWordIndex.find(str_Word) == map_DocOriginMapWordIndex.end())//没有该词语时
        {
            continue;
        }
        WordIndex* wordIndex = map_DocOriginMapWordIndex[str_Word];
        //wordIndex->Display();
        WordIndexRecord* wordIndexRecord =wordIndex->GetMapDocWordIndex()[docOrigin->GetDocID()];//单词索引的文档信息
        std::vector<WordPos> vec_Pos = wordIndexRecord->GetVecPos();
        //遍历索引信息，统计文档句子出现的位置，能有效减少合并范围的个数
        for(int iPos = 0; iPos<vec_Pos.size(); iPos++)
        {
            //更新句子出现的次数
            int senPos = vec_Pos[iPos].senPos;
            if(map_SentenceOccurTimes.find(senPos) == map_SentenceOccurTimes.end())
            {
                map_SentenceOccurTimes[senPos] = 1;
            }
            else
            {
                map_SentenceOccurTimes[senPos] = map_SentenceOccurTimes[senPos]+1;
            }
        }
    }
    return map_SentenceOccurTimes;
}

/**
    检测相似的句子是不是最长的
*//*
bool IsSimilarSentenceLongest(std::vector<TextRange>& vec_SenSearchTextRanges,TextRange textRange_Search)
{
    bool b_Longest = true;
    int n_SearchSenBegin = textRange_Search.offset;
    int n_SearchSenEnd = textRange_Search.offset + textRange_Search.length;
    for(int ivs=0; ivs<vec_SenSearchTextRanges.size(); ivs++)
    {
        TextRange textRange = vec_SenSearchTextRanges[ivs];
        int n_Begin = textRange.offset;
        int n_End = textRange.offset + textRange.length;
        //查看和保存的范围是否有交集，如果有再比较长度大小
        if((n_SearchSenBegin >= n_Begin && n_SearchSenBegin <=n_End) || (n_SearchSenEnd >=n_Begin && n_SearchSenEnd <= n_End))
        {
            if(textRange_Search.length < textRange.length )
            {
                b_Longest = false;
                break;
            }
            else if(textRange_Search.length > textRange.length)//如果长度比已经保存的句子的长度长，则删除保存过的句子范围
            {
                vec_SenSearchTextRanges.erase(vec_SenSearchTextRanges.begin()+ivs);
                ivs--;
            }
        }
    }
    return b_Longest;
}*/

/**
    对相同内容进行扩展匹配
    汉字的第一个字节的最高为为1
    英文最高位为0，且小于等于127
*/
void ExtendMatch(const std::string str_Search,int n_SearchSenOffset,TextRange& textrange_SearchDoc, \
                 const std::string str_Origin,int n_OriginSenOffset,TextRange& textrange_OriginDoc)
{
    //对第一处相同的指纹进行向前扩展匹配
    int n_SearchBegin = textrange_SearchDoc.offset-1;
    int n_OriginBegin = textrange_OriginDoc.offset-1;
    while(n_SearchBegin >=n_SearchSenOffset && n_OriginBegin>=n_OriginSenOffset \
            && str_Search[n_SearchBegin-n_SearchSenOffset] == str_Origin[n_OriginBegin-n_OriginSenOffset])
    {
        n_SearchBegin--;
        n_OriginBegin--;
    }
    //回到相等的位置
    n_SearchBegin++;
    n_OriginBegin++;
    //判断是否是一个字符的边界
    while(str_Search[n_SearchBegin-n_SearchSenOffset] > 127 && str_Origin[n_OriginBegin-n_OriginSenOffset] > 127)//既不是英文数字，也不是中文的第一个字节
    {
        n_SearchBegin++;
        n_OriginBegin++;
    }
    textrange_SearchDoc.length = textrange_SearchDoc.offset+textrange_SearchDoc.length - n_SearchBegin;
    textrange_SearchDoc.offset = n_SearchBegin;
    textrange_OriginDoc.length = textrange_OriginDoc.offset+textrange_OriginDoc.length - n_OriginBegin;
    textrange_OriginDoc.offset = n_OriginBegin;

    //对后面的字符向后扩展
    int n_SearchEnd = textrange_SearchDoc.offset + textrange_SearchDoc.length+1;
    int n_OriginEnd = textrange_OriginDoc.offset + textrange_OriginDoc.length+1;
    while(n_SearchEnd <=n_SearchSenOffset + str_Search.length() && n_OriginEnd <= n_OriginSenOffset + str_Origin.length() \
            && str_Search[n_SearchEnd-n_SearchSenOffset] == str_Origin[n_OriginEnd-n_OriginSenOffset])
    {
        n_SearchEnd++;
        n_OriginEnd++;
    }
    //回到相等的位置
    n_SearchEnd--;
    n_OriginEnd--;
    //判断是否是一个字符的边界
    while(str_Search[n_SearchEnd-n_SearchSenOffset] > 127 && str_Origin[n_OriginEnd-n_OriginSenOffset] > 127)//既不是英文数字，也不是中文的第一个字节
    {
        n_SearchEnd--;
        n_OriginEnd--;
    }
    textrange_SearchDoc.length = n_SearchEnd - textrange_SearchDoc.offset;
    textrange_OriginDoc.length = n_OriginEnd - textrange_OriginDoc.offset;
}

/**
    删除重复的范围
*/
void DeleteDuplicateTextRange(std::vector<TextRange>& vec_TextRanges)
{
    //删除相似范围内的重复
    for(int i=1; i<vec_TextRanges.size(); i++)
    {
        TextRange textrange = vec_TextRanges[i];
        TextRange textrangeLast = vec_TextRanges[i-1];
        if(textrange.offset < textrangeLast.offset+textrangeLast.length)
        {
            //当前的句子比上一个句子长，调整范围
            if(textrangeLast.offset + textrangeLast.length <= textrange.offset + textrange.length)
            {
                textrange.length = textrange.offset+textrange.length - textrangeLast.offset;
                textrange.offset = textrangeLast.offset;
            }else //当前的句子比上一个句子短
            {
                textrange.length = textrangeLast.offset+textrangeLast.length - textrange.offset;
            }
            if(textrange.n_Similar == TextSimilar && textrangeLast.n_Similar == TextSame)
            {
                textrange.n_Similar = TextSame;
            }
            vec_TextRanges.erase(vec_TextRanges.begin()+i-1);
            vec_TextRanges.erase(vec_TextRanges.begin()+i-1);
            vec_TextRanges.insert(vec_TextRanges.begin()+i-1,textrange);
            i--;
        }
    }
}

/**
    对范围排序
*/
bool TextRangeComparison(TextRange a,TextRange b)
{
    return a.offset<b.offset;
}

/**
    查询句子相似的文档
*/
void DocumentOperation::GetDiffer(Document* docOrigin,Document* docSearch)
{
    int n_SimilarNo = 1;//相似句子的编号
    //用来保存文本是否相同
    std::vector<TextRange> vec_DocSearchTextRanges;
    std::vector<TextRange> vec_DocOriginTextRanges;
    LongestSimilarSentence* lss = new LongestSimilarSentence();
    //doc2作为待检测文档,与doc1比较
    std::map<std::string,WordIndex*> map_DocOriginMapWordIndex = docOrigin->GetMapWordIndex();
    //挑选出现次数大于阈值的短语范围
    for(int i=0; i<docSearch->GetvecParagraph().size(); i++)
    {
        Paragraph para = docSearch->GetvecParagraph()[i];
        for(int j=0; j<para.vec_Sentences.size(); j++)
        {
            Sentence sen = para.vec_Sentences[j];
            std::vector<TextRange> vec_SenSearchTextRanges;//用于挑选最长相似的句子
            std::vector<TextRange> vec_SenOriginTextRanges;//用于挑选最长相似的句子
            if(sen.vec_splitedHits.size() < KGRAM)//词语数小于阈值时，不用处理
            {
                TextRange textRange;
                textRange.offset = sen.textRange.offset;
                textRange.length = sen.textRange.length;
                textRange.n_Similar = TextIgnore;
                vec_DocSearchTextRanges.push_back(textRange);
                continue;
            }
            std::string str_Search = docSearch->GetstrContents().substr(sen.textRange.offset,sen.textRange.length);
            std::map<int,int> map_SentenceOccurTimes = GetSentenceOccurTimes(sen,docOrigin,map_DocOriginMapWordIndex);
            //挑出包含N个相同词语的句子作为候选句子.
            for(std::map<int,int>::iterator it = map_SentenceOccurTimes.begin(); it != map_SentenceOccurTimes.end(); it++)
            {
                int times = it->second;
                if(times > SAMEWORDGATE)//相同词语的个数大于阈值
                {
                    int n_SenPos = it->first;
                    Sentence sen_Origin;
                    bool b_SentenceExists = docOrigin->GetSentenceByOffset(n_SenPos,sen_Origin);
                    if(!b_SentenceExists)
                    {
                        continue;
                    }
                    int length = sen_Origin.textRange.length;
                    int n_OriginDocBegin = n_SenPos;
                    int n_OriginDocEnd = n_SenPos + length;
                    //计算两个句子中相似的范围
                    std::vector<SenRangeSimilarity> vec_SenRangeSimilarity;
                    lss->GetSimBoundary(sen.vec_splitedHits,sen_Origin.vec_splitedHits,vec_SenRangeSimilarity);
                    //取出相似句子保存
                    for(int x=0; x<vec_SenRangeSimilarity.size(); x++)
                    {
                        SenRangeSimilarity senRangeSimilarity = vec_SenRangeSimilarity[x];
                        //查询的文档中的相似部分
                        Range range_SeachSen;
                        range_SeachSen.begin = sen.vec_splitedHits[senRangeSimilarity.range_SearchNo.begin].NoInDoc;
                        range_SeachSen.end = sen.vec_splitedHits[senRangeSimilarity.range_SearchNo.end].NoInDoc;
                        int n_SearchSenBegin = sen.vec_splitedHits[senRangeSimilarity.range_SearchNo.begin].textRange.offset;//相似句子段在文章中的偏移值
                        int n_SearchSenEnd = sen.vec_splitedHits[senRangeSimilarity.range_SearchNo.end].textRange.offset + sen.vec_splitedHits[senRangeSimilarity.range_SearchNo.end].textRange.length;
                        TextRange textrange_SearchDoc = {n_SearchSenBegin, n_SearchSenEnd - n_SearchSenBegin};//范围

                        // 原文件中相似的部分
                        Range range_OriginSen;
                        range_OriginSen.begin = sen_Origin.vec_splitedHits[senRangeSimilarity.range_SimilarNo.begin].NoInDoc;
                        range_OriginSen.end = sen_Origin.vec_splitedHits[senRangeSimilarity.range_SimilarNo.end].NoInDoc;
                        int n_OriginSenBegin = sen_Origin.vec_splitedHits[senRangeSimilarity.range_SimilarNo.begin].textRange.offset;//相似句子段在文章中的偏移值
                        int n_OriginSenEnd = sen_Origin.vec_splitedHits[senRangeSimilarity.range_SimilarNo.end].textRange.offset + sen_Origin.vec_splitedHits[senRangeSimilarity.range_SimilarNo.end].textRange.length;
                        TextRange textrange_OriginDoc = {n_OriginSenBegin, n_OriginSenEnd - n_OriginSenBegin};//范围

                        std::string str_Origin = docOrigin->GetstrContents().substr(sen_Origin.textRange.offset,sen_Origin.textRange.length);
                        //将前后位置比对，查看是否有相同的部分
                        ExtendMatch(str_Search,sen.textRange.offset,textrange_SearchDoc,str_Origin,sen_Origin.textRange.offset,textrange_OriginDoc);

                        //如果相似部分有重叠，只保留最长的部分
                        //bool b_Longest = IsSimilarSentenceLongest(vec_SenSearchTextRanges,textrange_SearchDoc);
                        //if(b_Longest)
                        {
                            //保存相似的部分
                            if(senRangeSimilarity.similarity == 1)
                            {
                                textrange_SearchDoc.n_Similar = TextSame;
                                textrange_OriginDoc.n_Similar = TextSame;
                            }
                            else
                            {
                                textrange_SearchDoc.n_Similar = TextSimilar;
                                textrange_OriginDoc.n_Similar = TextSimilar;
                            }
                            vec_SenSearchTextRanges.push_back(textrange_SearchDoc);
                            vec_SenOriginTextRanges.push_back(textrange_OriginDoc);
                        }
                        std::string str_SearchSimilar = docSearch->GetstrContents().substr(textrange_SearchDoc.offset,textrange_SearchDoc.length);
                        std::string str_OriginSimilar = docOrigin->GetstrContents().substr(textrange_OriginDoc.offset,textrange_OriginDoc.length);

/*
                        std::cout<<str_SearchSimilar<<std::endl;
                        std::cout<<"--------------"<<std::endl;
                        std::cout<<str_OriginSimilar<<std::endl<<std::endl;
                        //std::cin.get();
*/
                    }
                }
            }
            //将最终的相似句子加入到向量中
            if(vec_SenSearchTextRanges.size() == 0)//如果没有相似句子,则标记为不同
            {
                TextRange textRange;
                textRange.offset = sen.textRange.offset;
                textRange.length = sen.textRange.length;
                textRange.n_Similar = TextDifferent;
                vec_DocSearchTextRanges.push_back(textRange);
            }
            else
            {
                for(int is =0; is<vec_SenSearchTextRanges.size(); is++)
                {
                    TextRange textrange_SearchSen = vec_SenSearchTextRanges[is];
                    TextRange textrange_OriginSen = vec_SenOriginTextRanges[is];
                    textrange_SearchSen.number = n_SimilarNo;
                    n_SimilarNo++;
                    //前面不同的内容范围
                    vec_DocSearchTextRanges.push_back(textrange_SearchSen);
                    vec_DocOriginTextRanges.push_back(textrange_OriginSen);
                }
            }
        }
    }
    delete lss;
    sort(vec_DocSearchTextRanges.begin(),vec_DocSearchTextRanges.end(),TextRangeComparison);
    sort(vec_DocOriginTextRanges.begin(),vec_DocOriginTextRanges.end(),TextRangeComparison);
    DeleteDuplicateTextRange(vec_DocSearchTextRanges);
    DeleteDuplicateTextRange(vec_DocOriginTextRanges);
    docSearch->SetvecTextRange(vec_DocSearchTextRanges);
    docOrigin->SetvecTextRange(vec_DocOriginTextRanges);
}

DocumentOperation::~DocumentOperation()
{
    //dtor
}
