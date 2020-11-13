#include "Bitmap.h"

#include <immintrin.h> // AVX
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* init bitmap with parameters */
void Bitmap::init(int _window, int _memory, int _hashnum)
{ 
    window = _window;
    memory = _memory;
    hash_num = _hashnum;

    width = ((memory / sizeof(uint8_t)) / 16) * 16;

    updateLen = ((1 << (sizeof(uint8_t) * 8)) - 2) * width / window;
    max_counter_val = (1 << (sizeof(uint8_t) * 8)) - 1;
    lastUpdateIdx = 0;

    for(int i = 0; i < _hashnum; ++i)
        hash[i].initialize(rand() % MAX_PRIME32);
    hash_time_offset.initialize(rand() % MAX_PRIME32);

    memset(buckets, 0, sizeof(uint8_t) * MAX_CELL_NUM);
    printf("successfully initialize bitmap: winSize=%d, memory=%d, counterSize=%lu, updateLen=%d\n",
    	window, memory, sizeof(uint8_t), updateLen);
}

/* insert a item */
void Bitmap::insert(int x)
{
    for(int i = 0; i < hash_num; ++i){
        int pos = hash[i].run((char*)&x, sizeof(int)) % width;
        buckets[pos] = max_counter_val;
    }
}

/* update buckets */
void Bitmap::update(int insertTimesPerUpdate)
{
	int _updateLen = (updateLen * insertTimesPerUpdate / 32) * 32;
    int subAll = _updateLen / width;
    int len = _updateLen % width;

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

/* query the cardinality */
double Bitmap::query()
{
    double u = 0;
    for(int i = 0; i < width; ++i)
        u += buckets[i] == 0 ? 1 : 0;
    return -width * log(u / width);
}

/* update range using SIMD, end>=beg */
void Bitmap::update_range(int beg, int end, int val)
{
    if(val <= 0)    return;
    // for(int i = beg; i < end; ++i)
    //     buckets[i] = buckets[i] > val ? buckets[i] - val : 0;
    // return;

    /* address alignment */
	if(beg % 32 != 0){
		int endIdx = std::min(32 * (beg / 32 + 1), end);
		for(int i = beg; i < endIdx; ++i)
			buckets[i] = buckets[i] > val ? buckets[i] - val : 0;
		beg = endIdx;
	}

	__m256i _subVal = _mm256_set1_epi8(char(val));
	while(beg + 32 <= end){
		__m256i clock = _mm256_loadu_si256((__m256i*)&buckets[beg]);
		__m256i subRes = _mm256_subs_epu8(clock, _subVal);
		_mm256_storeu_si256((__m256i*)&buckets[beg], subRes);
		beg += 32;
	}

	/* deal with the left buckets */
	while(beg < end){
		buckets[beg] = buckets[beg] > val ? buckets[beg] - val : 0;
		beg++;
	}
}