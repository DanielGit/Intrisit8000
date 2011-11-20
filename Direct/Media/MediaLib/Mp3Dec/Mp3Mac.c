//[PROPERTY]===========================================[PROPERTY]
//            *****   电子词典平台  *****
//        --------------------------------------
//         	    MP3解码－模拟MAC单元部分
//        --------------------------------------
//                 版权: 新诺亚舟科技
//             ---------------------------
//                  版   本   历   史
//        --------------------------------------
//  版本    日前		说明		
//  V0.1    2005-8      Init.             Hisway.Gao
//[PROPERTY]===========================================[PROPERTY]
#include "Mp3Prv.h"

#ifdef CONFIG_DECODE_MP3_ENABLE

#if defined(USE_MAC) && defined(WIN32)

static __int64 _MacAcc;
static int *_pMacMultiplicand;
static int _MacMode = 0;
static int _ModeShift = 0;

////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
void MacInit(int mode, int acc)
{
	_MacAcc = (__int64)acc;
	_MacMode = mode % 3;
	switch(_MacMode)
	{
	case 0:
		_ModeShift = 31;
		break;
	case 1:
		_ModeShift = 32;
		break;
	case 2:
		_ModeShift = 0;
		break;
	}
}

////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
void MacAddr(int *addr)
{
	_pMacMultiplicand = addr;
}

////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
int MacResult(void)
{
	return (int)(_MacAcc >> _ModeShift);
}


////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
void MacFour(int m0, int m1, int m2, int m3)
{
	_MacAcc += (__int64)m0 * (__int64)(*_pMacMultiplicand++);
	_MacAcc += (__int64)m1 * (__int64)(*_pMacMultiplicand++);
	_MacAcc += (__int64)m2 * (__int64)(*_pMacMultiplicand++);
	_MacAcc += (__int64)m3 * (__int64)(*_pMacMultiplicand++);
}

////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
void MacOne(int m0)
{
	_MacAcc += (__int64)m0 * (__int64)(*_pMacMultiplicand++);
}

////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
int MacRepeat (unsigned int times, int *HeadADD_1, int *HeadADD_2)
{
	_pMacMultiplicand = HeadADD_2;

	while(times > 0)
	{
		_MacAcc += ((__int64)(*HeadADD_1++)) * ((__int64)(*_pMacMultiplicand++));
		times--;
	}
	
	return (int)(_MacAcc >> _ModeShift);
}

////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
int MacRepeat8 (unsigned int times, int *HeadADD_1, int *HeadADD_2)
{
	_pMacMultiplicand = HeadADD_2;

	_MacAcc += ((__int64)(*HeadADD_1++)) * ((__int64)(*_pMacMultiplicand++));
	_MacAcc += ((__int64)(*HeadADD_1++)) * ((__int64)(*_pMacMultiplicand++));
	_MacAcc += ((__int64)(*HeadADD_1++)) * ((__int64)(*_pMacMultiplicand++));
	_MacAcc += ((__int64)(*HeadADD_1++)) * ((__int64)(*_pMacMultiplicand++));
	_MacAcc += ((__int64)(*HeadADD_1++)) * ((__int64)(*_pMacMultiplicand++));
	_MacAcc += ((__int64)(*HeadADD_1++)) * ((__int64)(*_pMacMultiplicand++));
	_MacAcc += ((__int64)(*HeadADD_1++)) * ((__int64)(*_pMacMultiplicand++));
	_MacAcc += ((__int64)(*HeadADD_1++)) * ((__int64)(*_pMacMultiplicand++));

	return (int)(_MacAcc >> _ModeShift);
}


////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
int MacRepeat16 (unsigned int times, int *HeadADD_1, int *HeadADD_2)
{
	_pMacMultiplicand = HeadADD_2;

	_MacAcc += ((__int64)(*HeadADD_1++)) * ((__int64)(*_pMacMultiplicand++));
	_MacAcc += ((__int64)(*HeadADD_1++)) * ((__int64)(*_pMacMultiplicand++));
	_MacAcc += ((__int64)(*HeadADD_1++)) * ((__int64)(*_pMacMultiplicand++));
	_MacAcc += ((__int64)(*HeadADD_1++)) * ((__int64)(*_pMacMultiplicand++));
	_MacAcc += ((__int64)(*HeadADD_1++)) * ((__int64)(*_pMacMultiplicand++));
	_MacAcc += ((__int64)(*HeadADD_1++)) * ((__int64)(*_pMacMultiplicand++));
	_MacAcc += ((__int64)(*HeadADD_1++)) * ((__int64)(*_pMacMultiplicand++));
	_MacAcc += ((__int64)(*HeadADD_1++)) * ((__int64)(*_pMacMultiplicand++));

	_MacAcc += ((__int64)(*HeadADD_1++)) * ((__int64)(*_pMacMultiplicand++));
	_MacAcc += ((__int64)(*HeadADD_1++)) * ((__int64)(*_pMacMultiplicand++));
	_MacAcc += ((__int64)(*HeadADD_1++)) * ((__int64)(*_pMacMultiplicand++));
	_MacAcc += ((__int64)(*HeadADD_1++)) * ((__int64)(*_pMacMultiplicand++));
	_MacAcc += ((__int64)(*HeadADD_1++)) * ((__int64)(*_pMacMultiplicand++));
	_MacAcc += ((__int64)(*HeadADD_1++)) * ((__int64)(*_pMacMultiplicand++));
	_MacAcc += ((__int64)(*HeadADD_1++)) * ((__int64)(*_pMacMultiplicand++));
	_MacAcc += ((__int64)(*HeadADD_1++)) * ((__int64)(*_pMacMultiplicand++));

	return (int)(_MacAcc >> _ModeShift);
}
#endif


#endif // CONFIG_DECODE_MP3_ENABLE

