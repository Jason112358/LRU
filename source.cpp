#include <iostream>
#include <ctime>
#include <fstream>
#include "LRU.cpp"
using namespace std;

int MAX_FRAME; 
const int MAX_PAGE_NUM = 10; 
const int TEST_PAGE_NUM = 3000; 
int missingNum[4] = { 0 };

LRU_Ctr *LRU_Ctr_list;
LRU_Stk *LRU_Stk_list;
LRU_Arb *LRU_Arb_list;
LRU_Sec *LRU_Sec_list;

void Replace_LRU_Ctr(int, int);
void Replace_LRU_Arb(int);

ifstream ofs;

typedef enum
{
	LRU_C, LRU_S, ARB, SC
};

int main()
{
	char File[255];
	cout << "Name of the file:  ";
	cin >> File;
	ofs.open(File, ios::in);
	cout << "The max number of frames: ";
	cin >> MAX_FRAME;

	if (MAX_FRAME <= 0) exit(1);//In case of the risk

	//Initialization
	LRU_Ctr_list = new LRU_Ctr[MAX_FRAME];
	LRU_Stk_list = new LRU_Stk(MAX_FRAME);
	LRU_Arb_list = new LRU_Arb[MAX_FRAME];
	LRU_Sec_list = new LRU_Sec(MAX_FRAME);

	//Beginning
	int access_page_id;
	int i;
	char rw;
	clock_t Ctr_start, Ctr_stop, Stk_start, Stk_stop, Arb_start, Arb_stop, Sec_start, Sec_stop;
	double Ctr_time = 0, Stk_time = 0, Arb_time = 0, Sec_time = 0;
	for (i = 0; ofs.eof() == false; i++)
	{
		ofs >> hex >> access_page_id >> rw;
		
		//LRU_Counter
		Ctr_start = clock();
		Replace_LRU_Ctr(access_page_id, i);
		Ctr_stop = clock();
		Ctr_time += (double)(Ctr_stop - Ctr_start) / CLOCKS_PER_SEC;

		//LRU_Stack
		Stk_start = clock();
		bool have_fault = LRU_Stk_list->TryAccess(access_page_id);
		if (have_fault) missingNum[LRU_S]++;
		Stk_stop = clock();
		Stk_time += (double)(Stk_stop - Stk_start) / CLOCKS_PER_SEC;

		//LRU-ARB
		Arb_start = clock();
		Replace_LRU_Arb(access_page_id);
		Arb_stop = clock();
		Arb_time += (double)(Arb_stop - Arb_start) / CLOCKS_PER_SEC;

		//LRU-Second
		Sec_start = clock();
		have_fault = LRU_Sec_list->TryAccess(access_page_id);
		if (have_fault) missingNum[SC] ++;
		Sec_stop = clock();
		Sec_time += (double)(Sec_stop - Sec_start) / CLOCKS_PER_SEC;

		if (i % 2000000 == 0 && i!=0) cout << "我还在跑，刚跑完" << i << "条 Zzzz.........\n";
	}
	cout << "\n我跑完了，结果在下面，让我睡觉吧...........\n";
	cout << "LRU_Ctr = " << missingNum[LRU_C] << "/" << (i + 1) << " = " << missingNum[LRU_C] / (float)(i + 1) << "\tTime: " << Ctr_time << endl;
	cout << "LRU_Stk = " << missingNum[LRU_S] << "/" << (i + 1) << " = " << missingNum[LRU_S] / (float)(i + 1) << "\tTime: " << Stk_time << endl;
	cout << "LRU_Arb = " << missingNum[ARB] << "/" << (i + 1) << " = " << missingNum[ARB] / (float)(i + 1) << "\tTime: " << Arb_time << endl;
	cout << "LRU_Sec = " << missingNum[SC] << "/" << (i + 1) << " = " << missingNum[SC] / (float)(i + 1) << "\tTime: " << Sec_time << endl;

	ofs.close();

	system ("pause");
	return 0;
}

void Replace_LRU_Ctr(int pageID, int timeCount)
{
	for (int i = 0; i < MAX_FRAME; i++)
	{
		LRU_Ctr *lruFrame = &(LRU_Ctr_list[i]);
		if (lruFrame->ID == pageID)
		{
			lruFrame->counter = timeCount;
			return;
		}
	}
	missingNum[LRU_C] ++; 
	int min_time_count = 65535;
	int min_index = -1;
	for (int i = 0; i < MAX_FRAME; i++)
	{
		LRU_Ctr *lruFrame = &(LRU_Ctr_list[i]);
		if (lruFrame->ID == -1)
		{
			lruFrame->ID = pageID;
			lruFrame->counter = timeCount;
			return;
		}
		if (lruFrame->counter < min_time_count)
		{
			min_time_count = lruFrame->counter;
			min_index = i;
		}
	}
	//replace
	LRU_Ctr_list[min_index].ID = pageID;
	LRU_Ctr_list[min_index].counter = timeCount;
}

void Replace_LRU_Arb(int pageID)
{
	for (int i = 0; i < MAX_FRAME; i++)
	{
		LRU_Arb *lruFrame = &(LRU_Arb_list[i]);
		lruFrame->MoveRight();
	}
	for (int i = 0; i < MAX_FRAME; i++)
	{
		LRU_Arb *lruFrame = &(LRU_Arb_list[i]);
		if (lruFrame->ID == pageID)
		{
			lruFrame->Access();
			return; 
		}
	}
	missingNum[ARB] ++; //Increase page fault count
	int min_ref_val = 0x100;;
	int min_index = -1;
	for (int i = 0; i < MAX_FRAME; i++)
	{
		LRU_Arb *lruFrame = &(LRU_Arb_list[i]);
		if (lruFrame->ID == -1)
		{
			lruFrame->ID = pageID;
			lruFrame->Access();
			return;
		}
		if (lruFrame->refBits < min_ref_val)
		{
			min_ref_val = lruFrame->refBits;
			min_index = i;
		}
	}
	//replace
	LRU_Arb_list[min_index].ID = pageID;
	LRU_Arb_list[min_index].Access();
}