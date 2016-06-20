//////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2014 Gefu Tang <tanggefu@gmail.com> & Yang Long <20288ly@sina.cn>.
/// All Rights Reserved.
///
/// This file is part of LSHBOX.
///
/// LSHBOX is free software: you can redistribute it and/or modify it under
/// the terms of the GNU General Public License as published by the Free
/// Software Foundation, either version 3 of the License, or(at your option)
/// any later version.
///
/// LSHBOX is distributed in the hope that it will be useful, but WITHOUT
/// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
/// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
/// more details.
///
/// You should have received a copy of the GNU General Public License along
/// with LSHBOX. If not, see <http://www.gnu.org/licenses/>.
///
/// @version 0.9
/// @author Gefu Tang, Yang Long &  Zhifeng Xiao
/// @date 2015.5.18
//////////////////////////////////////////////////////////////////////////////

/**
 * @file topk.h
 *
 * @brief Top-K data structures.
 */
#pragma once
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <algorithm>
namespace lshbox
{
/**
 * Max Heap.
 *
 * This is a max heap for TopK.
 */
template<typename Comparable>
class MaxHeap
{
public:
    explicit MaxHeap(int capacity = 100): array(capacity + 1), currentSize(0) {}
    explicit MaxHeap(const std::vector<Comparable> &items): array(items.size() + 10), currentSize(items.size())
    {
        for (int i = 0; i < items.size(); ++i)
        {
            array[i + 1] = items[i];
        }
        buildHeap();
    }
    /**
     * check if the heap is empty.
     */
    bool isEmpty() const
    {
        return currentSize == 0;
    }
    /**
     * get the max value.
     */
    const Comparable & findMax() const
    {
        if (isEmpty())
        {
            std::cout << "UnderflowException() ..." << std::endl;
        }
        return array[1];
    }
    /**
     * inser a value.
     * @param x the insert value.
     */
    void insert(const Comparable & x)
    {
        if (currentSize == array.size() - 1)
        {
            array.resize(array.size() * 2);
        }
        int hole = ++currentSize;
        for (; hole > 1 && x > array[hole / 2]; hole /= 2)
        {
            array[hole] = array[hole / 2];
        }
        array[hole] = x;
    }
    /**
     * delete the max value.
     */
    void deleteMax()
    {
        if (isEmpty())
        {
            std::cout << "UnderflowException() ..." << std::endl;
        }
        array[1] = array[currentSize--];
        percolateDown(1);
    }
    /**
     * delete the max value.
     * @param minItem the max value.
     */
    void deleteMax(Comparable &maxItem)
    {
        if (isEmpty())
        {
            std::cout << "UnderflowException() ..." << std::endl;
        }
        maxItem = array[1];
        array[1] = array[currentSize--];
        percolateDown(1);
    }
    /**
     * make the heap empty.
     */
    void makeEmpty()
    {
        currentSize = 0;
    }
    /**
     * the size of max heap.
     */
    int size()
    {
        return currentSize;
    }

private:
    int currentSize;
    std::vector<Comparable> array;
    /**
     * build heap.
     */
    void buildHeap()
    {
        for (int i = currentSize / 2; i > 0; --i)
        {
            percolateDown(i);
        }
    }
    /**
     * percolate down the binary heap.
     * @param hole the index.
     */
    void percolateDown(int hole)
    {
        int child;
        Comparable tmp = array[hole];
        for (; hole * 2 <= currentSize; hole = child)
        {
            child = hole * 2;
            if (child != currentSize && array[child + 1] > array[child])
            {
                child++;
            }
            if (array[child] > tmp)
            {
                array[hole] = array[child];
            }
            else
            {
                break;
            }
        }
        array[hole] = tmp;
    }
};

/**
 * Top-K heap.
 *
 * At this point topk should contain the nearest k query keys and distances.
 */
class Topk
{
private:
    unsigned K;
    MaxHeap<std::pair<float, unsigned> > heap;
    std::vector<std::pair<float, unsigned> > tops;
public:
    Topk(): K(0) {}
    /**
     * reset K value.
     * @param _K the K value in TopK.
     */
    void reset(int _K)
    {
        K = _K;
        tops.resize(0);
    }
    /**
     * push a value into the maxHeap.
     * @param key  the key.
     * @param dist the distance.
     */
    void push(unsigned key, float dist)
    {
        std::pair<float, unsigned> item(dist, key);
        unsigned S = heap.size();
        if (S < K)
        {
            heap.insert(item);
        }
        else if (item < heap.findMax())
        {
            heap.deleteMax();
            heap.insert(item);
        }
    }
    /**
     * generate TopK.
     */
    void genTopk()
    {
        unsigned total = heap.size();
        for (unsigned i = 0; i != total; ++i)
        {
            std::pair<float, unsigned> top;
            heap.deleteMax(top);
            tops.push_back(top);
        }
        std::reverse(tops.begin(), tops.end());
    }
    /**
     * Get the std::vector<std::pair<float, unsigned> > instance which contains the nearest keys and distances.
     */
    const std::vector<std::pair<float, unsigned> > &getTopk() const
    {
        return tops;
    }
    /**
     * Get the std::vector<std::pair<float, unsigned> > instance which contains the nearest keys and distances.
     */
    std::vector<std::pair<float, unsigned> > &getTopk()
    {
        return tops;
    }
    /**
     * Calculate the recall vale with another heap.
     * @param  topk another TopK.
     */
    const float recall(const Topk &topk) const
    {
        std::vector<std::pair<float, unsigned> > tops = getTopk();
        std::vector<std::pair<float, unsigned> > benchTops = topk.getTopk();
        unsigned matched = 0;
        for (std::vector<std::pair<float, unsigned> >::iterator i = tops.begin(); i != tops.end(); ++i)
        {
            for (std::vector<std::pair<float, unsigned> >::iterator j = benchTops.begin(); j != benchTops.end(); ++j)
            {
                if (i->second == j->second)
                {
                    ++matched;
                    break;
                }
            }
        }
        return float(matched + 1) / float(benchTops.size() + 1);
    }
};

/**
 * Top-K scanner.
 *
 * Scans keys for top-K query, this is the object passed into the LSH query interface.
 */
template<typename ACCESSOR>
class Scanner
{
public:
    typedef typename ACCESSOR::Value Value;
    typedef typename ACCESSOR::DATATYPE DATATYPE;
    /**
     * Constructor for this class.
     *
     * @param accessor The scanner use accessor to retrieva values from keys.
     * @param metric The distance metric.
     * @param K Value used to reset internal Topk class.
     * @param R Value used to reset internal Topk class.
     */
    Scanner(
        const ACCESSOR &accessor,
        const Metric<DATATYPE> &metric,
        unsigned K
    ): accessor_(accessor), metric_(metric), K_(K), cnt_(0) {}
    void resetK(unsigned K)
    {
        if (K_ != K)
        {
            K_ = K;
            topk_.reset(K_);
        }
    }
    /**
      * Reset the query, this function should be invoked before each query.
      */
    void reset(Value query)
    {
        query_ = query;
        accessor_.reset();
        topk_.reset(K_);
        cnt_ = 0;
    }
    /**
     * Number of points scanned for the current query.
     */
    unsigned cnt() const
    {
        return cnt_;
    }
    /**
     * TopK results.
     */
    const Topk &topk() const
    {
        return topk_;
    }
    /**
     * TopK results.
     */
    Topk &topk()
    {
        return topk_;
    }
    /**
     * Update the current query by scanning key, this is normally invoked by the LSH
     * index structure.
     */
    void operator () (unsigned key)
    {
        if (accessor_.mark(key))
        {
            ++cnt_;
            topk_.push(key, metric_.dist(query_, accessor_(key)));
        }
    }
private:
    ACCESSOR accessor_;
    Metric<DATATYPE> metric_;
    Topk topk_;
    Value query_;
    unsigned K_;
    unsigned cnt_;
};

template<typename DATATYPE>
class FilesScanner
{
public:
    FilesScanner() {}
    FilesScanner(
        std::vector<std::map<std::string, std::vector<unsigned> > > &tables_,
        std::vector<std::map<std::string, std::pair<std::string, unsigned> > > &hashPos_,
        std::vector<std::map<std::string, unsigned> > &fileSize_,
        unsigned N_,
        unsigned dim_,
        unsigned maxFilesNum_,
        std::string hashSavePath_,
        const Metric<DATATYPE> &metric,
        unsigned K
    ): tables(tables_), hashPos(hashPos_), fileSize(fileSize_), maxFilesNum(maxFilesNum_), N(N_), dim(dim_), hashSavePath(hashSavePath_), metric_(metric), K_(K), cnt_(0)
    {
        flags_.resize(N);
        // fillFilesDB();
    }
    void init(
        std::vector<std::map<std::string, std::vector<unsigned> > > &tables_,
        std::vector<std::map<std::string, std::pair<std::string, unsigned> > > &hashPos_,
        std::vector<std::map<std::string, unsigned> > &fileSize_,
        unsigned N_,
        unsigned dim_,
        unsigned maxFilesNum_,
        std::string hashSavePath_,
        const Metric<DATATYPE> &metric,
        unsigned K
    )
    {
        tables = tables_;
        hashPos = hashPos_;
        fileSize = fileSize_;
        maxFilesNum = maxFilesNum_;
        N = N_;
        dim = dim_;
        hashSavePath = hashSavePath_;
        metric_ = metric;
        K_ = K;
        cnt_ = 0;
        flags_.resize(N);
        // fillFilesDB();
    }
    ~FilesScanner()
    {
        for (auto iter = filesDB.begin(); iter != filesDB.end(); ++iter)
        {
            if (iter->second != NULL)
            {
                delete [] iter->second;
            }
        }
        filesDB.clear();
    }
    void resetK(unsigned K)
    {
        if (K_ != K)
        {
            K_ = K;
            topk_.reset(K_);
        }
    }
    void reset(DATATYPE *query)
    {
        query_ = query;
        topk_.reset(K_);
        cnt_ = 0;
        flags_.clear();
        flags_.resize(N);
    }
    void fillFilesDB()
    {
        unsigned cnt = 0;
        for (unsigned table_id = 0; table_id != hashPos.size(); ++table_id)
        {
            for (auto iter = hashPos[table_id].begin(); iter != hashPos[table_id].end(); ++iter)
            {
                std::string file = iter->first;
                auto key = std::make_pair(table_id, file);
                filesDB[key] = new DATATYPE[fileSize[table_id][file] * dim];
                std::ifstream in(hashSavePath + "/L_" + std::to_string(long double(table_id)) + "/" + file + ".hash", std::ios::binary);
                in.read((char *)filesDB[key], fileSize[table_id][file] * dim * sizeof(DATATYPE));
                if (++cnt == maxFilesNum)
                {
                    return;
                }
            }
        }
    }
    /**
     * Number of points scanned for the current query.
     */
    unsigned cnt() const
    {
        return cnt_;
    }
    /**
     * TopK results.
     */
    const Topk &topk() const
    {
        return topk_;
    }
    /**
     * TopK results.
     */
    Topk &topk()
    {
        return topk_;
    }
    DATATYPE *useFile(unsigned table_id, std::string hashVal)
    {
        std::string file = hashPos[table_id][hashVal].first;
        auto key = std::make_pair(table_id, file);
        auto iter = filesDB.find(key);
        if (iter == filesDB.end())
        {
            if (filesDB.size() == maxFilesNum)
            {
                if (filesDB.begin()->second != NULL)
                {
                    delete [] filesDB.begin()->second;
                }
                filesDB.erase(filesDB.begin());
            }
            filesDB[key] = new DATATYPE[fileSize[table_id][file] * dim];
            std::ifstream in(hashSavePath + "/L_" + std::to_string(long double(table_id)) + "/" + file + ".hash", std::ios::binary);
            in.read((char *)filesDB[key], fileSize[table_id][file] * dim * sizeof(DATATYPE));
        }
        return filesDB[key];
    }
    bool mark(unsigned key)
    {
        if (flags_[key])
        {
            return false;
        }
        flags_[key] = true;
        return true;
    }
    void insert(unsigned table_id, std::string hashVal)
    {
        DATATYPE *vecs = useFile(table_id, hashVal);
        std::vector<unsigned> &keys = tables[table_id][hashVal];
        unsigned &pos = hashPos[table_id][hashVal].second;
        for (unsigned i = 0; i != keys.size(); ++i)
        {
            if (mark(keys[i]))
            {
                ++cnt_;
                topk_.push(keys[i], metric_.dist(query_, vecs + (pos + i) * dim));
            }
        }
    }
private:
    Metric<DATATYPE> metric_;
    Topk topk_;
    DATATYPE *query_;
    unsigned K_;
    unsigned cnt_;
    std::vector<bool> flags_;
    std::map<std::pair<unsigned, std::string>, DATATYPE *> filesDB;
    unsigned N, dim, maxFilesNum;
    std::string hashSavePath;
    std::vector<std::map<std::string, std::vector<unsigned> > > tables;
    std::vector<std::map<std::string, std::pair<std::string, unsigned> > > hashPos;
    std::vector<std::map<std::string, unsigned> > fileSize;
};
}