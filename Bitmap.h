#ifndef _BITMAP_H_
#define _BITMAP_H_

#include "BOBHash32.h"

#include <immintrin.h> // AVX
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

class Bitmap
{
    static const int MAX_CELL_NUM = 32 * 1e5 * 5;
    static const int MAX_HASH_NUM = 50;
    int window;
    int memory;
    int hash_num;
    int width;
    int updateLen;
    int max_counter_val;
    BOBHash32 hash[MAX_HASH_NUM];
    BOBHash32 hash_time_offset;
    alignas(64) uint8_t buckets[MAX_CELL_NUM];
    int lastUpdateIdx;
public:
    void init(int _window, int _memory, int insertTimesPerUpdate, int _hashnum=1)
    {
        for(int i = 0; i < _hashnum; ++i)
            hash[i].initialize(rand() % MAX_PRIME32);
        hash_time_offset.initialize(rand() % MAX_PRIME32);

        window = _window;
        memory = _memory;
        hash_num = _hashnum;
        width = ((memory / sizeof(uint8_t)) / 16) * 16;
        updateLen = ((1 << sizeof(uint8_t)) - 2) * width / window * insertTimesPerUpdate;
        updateLen = (updateLen / 32) * 32;
        max_counter_val = (1 << sizeof(uint8_t)) - 1;
        lastUpdateIdx = 0;

        memset(buckets, 0, sizeof(unsigned short) * MAX_CELL_NUM);
        printf("successfully initialize bitmap: winSize=%d, memory=%d, counterSize=%d, insertTimesPerUpdate=%d",
            window, memory, sizeof(uint8_t), insertTimesPerUpdate);
    }
    void insert(int x)
    {
        for(int i = 0; i < hash_num; ++i){
            int pos = hash[i].run((char*)&x, sizeof(int)) % width;
            buckets[pos] = max_counter_val;
        }
    }
    void update()
    {
        int subAll = updateLen / width;
        int len = updateLen % width;

        int beg = lastUpdateIdx, end = std::min(width, lastUpdateIdx + len);
        update_range(beg, end, subAll + 1);
        if(end - beg < len)
        {
            end = len - (end - beg);
            beg = 0;
            update_range(beg, end, subAll + 1);
        }

        if(end > lastUpdateIdx){
            update_range(end, width, subAll);
            update_range(0, lastUpdateIdx, subAll);
        }
        else
            update_range(end, lastUpdateIdx, subAll);
        lastUpdateIdx = end;
    }
    double query()
    {
        double u = 0;
        for_each(buckets, buckets + width, [&u](unsigned short &x){ u += (x == 0 ? 1 : 0); });
        return -width * log(u / width);
    }
private:
    void update_range(int beg, int end, int val)
    {
        if(val <= 0)    return;
        for(int i = beg; i < end; ++i)
            buckets[i] = buckets[i] > val ? buckets[i] - val : 0;

        // /* address alignment */
		// if(beg % 32 != 0){
		// 	int endIdx = std::min(32 * (beg / 32 + 1), end);
		// 	for(int i = beg; i < endIdx; ++i)
		// 		buckets[i] = buckets[i] > val ? buckets[i] - val : 0;
		// 	beg = endIdx;
		// }

		// __m256i _subVal = _mm256_set1_epi16(short(val));
		// while(beg + 32 <= end){
		// 	__m256i clock = _mm256_loadu_si256((__m256i*)&buckets[beg]);
		// 	__m256i subRes = _mm256_subs_epu16(clock, _subVal);
		// 	_mm256_storeu_si256((__m256i*)&buckets[beg], subRes);
		// 	beg += 32;
		// }

		// /* deal with the left buckets */
		// while(beg < end){
		// 	buckets[beg] = buckets[beg] > val ? buckets[beg] - val : 0;
		// 	beg++;
		// }
    }
};

#endif //_BITMAP_H_