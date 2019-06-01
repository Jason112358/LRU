#include <iostream>
using namespace std;

struct LRU_Ctr
{
	int counter;
	int ID;
	LRU_Ctr()
	{
		ID = -1;
		counter = 65536;
	}
};

struct LRU_Arb
{
	unsigned char refBits;
	int ID;
	LRU_Arb()
	{
		ID = -1; 
		refBits = 0x0;
	}
	void MoveRight()
	{
		refBits >>= 1;
	}
	void Access()
	{
		refBits |= 0x80;
	}
};

struct LRU_s_Item
{
	int ID;
	LRU_s_Item *head;
	LRU_s_Item *tail;
	LRU_s_Item()
	{
		ID = -1;
		head = NULL;
		tail = NULL;
	}
};

struct LRU_Stk
{
	LRU_s_Item *pFront; //Õ»¶¥Ö¸Õë
	LRU_s_Item *pEnd; //Õ»µ×Ö¸Õë
	int capacity;
	int size;
	LRU_Stk(int Cap)
	{
		capacity = Cap;
		size = 0;
		pFront = NULL;
		pEnd = NULL;
	}
	LRU_s_Item* Push(int pageID)
	{
		if (size >= capacity)
		{
			//Need replace...
			return NULL;
		}
		LRU_s_Item *newHeadItem = new LRU_s_Item;
		newHeadItem->ID = pageID;
		newHeadItem->tail = pFront;

		if (pFront != NULL) pFront->head = newHeadItem;
		if (pEnd == NULL) pEnd = newHeadItem;

		pFront = newHeadItem;
		size++;

		return pFront;
	}
	LRU_s_Item* Replace(int pageID)
	{
		LRU_s_Item *oldEnd = pEnd;
		pEnd = pEnd->head;
		if (pEnd != NULL)
			pEnd->tail = NULL;
		size--;
		return Push(pageID);
	}
	bool TryAccess(int pageID)
	{
		LRU_s_Item * pItem = pFront;
		while (pItem != NULL)
		{
			if (pageID == pItem->ID)
			{
				if (pItem == pFront)
					return false;
					pItem->head->tail =
					pItem->tail;
					if (pItem->tail != NULL) pItem->tail->head = pItem->head;
					else pEnd = pItem->head;
				pFront->head = pItem;
				pItem->head = NULL;
				pItem->tail = pFront;
				pFront = pItem;

				return false;
			}
			pItem = pItem->tail;
		}
		if (size < capacity)
		{
			Push(pageID);
			return true;
		}
		Replace(pageID);
		return true;
	}
};

struct LRU_Sec_frame
{
	bool ref_bit;
	int ID;
	LRU_Sec_frame *pNext;
	LRU_Sec_frame()
	{
		ref_bit = false;
		ID = -1;
	}
};

struct LRU_Sec
{
	//Second-chance Page-rep. Algorithm
	LRU_Sec_frame *pHead;
	LRU_Sec_frame *pLastAccess;
	int capacity;
	int size;
	LRU_Sec(int _capacity)
	{
		capacity = _capacity;
		size = 0;
		pHead = NULL;
		pLastAccess = NULL;
	}
	LRU_Sec_frame* Insert(int pageID)
	{
		if (size >= capacity)
		{
			cerr << "Container oversize!" << endl;
				return NULL;
		}
		LRU_Sec_frame *pItem = pHead;
		LRU_Sec_frame *newFrame = new LRU_Sec_frame;
		newFrame->ID = pageID;
		if (pItem == NULL)
		{
			newFrame->pNext = newFrame;
			pHead = newFrame;
			size++;
			return newFrame;
		}
		while (pItem->pNext != pHead) pItem = pItem->pNext;

		newFrame->pNext = pHead;
		pItem->pNext = newFrame;
		size++;

		return newFrame;
	}

	void Replace(LRU_Sec_frame *pFrame, int pID)
	{
		pFrame->ID = pID;
		pFrame->ref_bit = true;
	}

	bool TryAccess(int pageID)
	{
		LRU_Sec_frame *pItem = pLastAccess;
		if (pItem == NULL)
		{
			pItem = Insert(pageID);
			pItem->ref_bit = true;
			pHead = pItem;
			pLastAccess = pItem;
			return true;
		}
		do
		{
			if (pageID == pItem->ID)
			{
				pItem->ref_bit = true;
				pLastAccess = pItem;
				return false;
			}
			pItem = pItem->pNext;
		} while (pItem != pLastAccess);

		if (size < capacity)
		{
			pItem = Insert(pageID);
			pItem->ref_bit = true;
			pLastAccess = pItem;
			return true;
		}

		pItem = pLastAccess;
		while (true)
		{
			if (pItem->ref_bit == false)
			{
				Replace(pItem, pageID);
				pLastAccess = pItem;
				return true;
			}
			else
			{
				pItem->ref_bit = false;
			}
			pItem = pItem->pNext;
		}
	}
};