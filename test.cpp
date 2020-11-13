#include <stdio.h>
#include <time.h>
#include <fstream>
#include <unordered_map>
#include <unordered_set>

#include "Bitmap.h"

using namespace std;

const static int MAX_PACKET_NUM = 3e7 + 10;
static int flow[MAX_PACKET_NUM];
void load_data()
{
    BOBHash32 hash_id(rand() % MAX_PRIME32);
    ifstream input("formatted00.dat", ios::in | ios::binary);
    char buf[2000] = {0};
    int packet_cnt;
    for(packet_cnt = 0; packet_cnt < MAX_PACKET_NUM; ++packet_cnt)
    {
        if(!input.read(buf, 16)){
            printf("ERROR   %d\n", packet_cnt);
            break;
        }
        flow[packet_cnt] = hash_id.run(buf, 8);
    }
    printf("read in %d packets\n", packet_cnt);
}


void test_bitmap()
{
    Bitmap bitmap;
    unordered_set<int> inSet;
    const static int insertTimesPerUpdate = 10;
    
    for (int win = (1 << 12); win <= (1 << 14); win <<= 1) 
        for (int mem = (1 << 16); mem <= (1 <<16); mem <<= 1)
        {
            bitmap.init(win, mem, insertTimesPerUpdate);
            for(int i = 0; i + 10 <= win * 3; i += insertTimesPerUpdate){   // insert 3 wins
                for(int j = i; j < i + 10; ++j)
                    bitmap.insert(flow[j]);
                bitmap.update();
            }

            for(int iWin = 3; iWin < 15; ++iWin)        // insert a win each time
            {   
                inSet.clear();
                for(int i = iWin * win; i + 10 <= (iWin + 1) * win; i += insertTimesPerUpdate){   // insertion
                    for(int j = i; j < i + 10; ++j){
                        bitmap.insert(flow[j]);
                        inSet.insert(flow[j]);
                    }
                    bitmap.update();
                }

                double bmCard = bitmap.query();
                double cr = bmCard / inSet.size();
                printf("query time range (%d-%d):\trelatedError:%.6lf\tbmCard:%.6lf\trealCard:%.6lf\n",
                    iWin * win, (iWin + 1) * win, fabs(cr - 1), bmCard, inSet.size());
            }
        }
}

int main()
{
    srand(clock());
    load_data();
    test_bitmap();

    return EXIT_SUCCESS;
}