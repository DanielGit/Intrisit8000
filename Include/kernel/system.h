//[PROPERTY]===========================================[PROPERTY]
//            *****   ���Ӵʵ��ļ�ϵͳ  *****
//        --------------------------------------
//                    �ں����ͷ�ļ�
//        --------------------------------------
//                 ��Ȩ: ��ŵ���ۿƼ�
//             ---------------------------
//                  ��   ��   ��   ʷ
//        --------------------------------------
//  �汾    ��ǰ		˵��
//  V0.1    2007-4      Init.             Hisway.Gao
//[PROPERTY]===========================================[PROPERTY]


#ifndef _SYSTEM_H
#define _SYSTEM_H

#if defined(STC_EXP)
int sKernelInfo(int, void *);
int sKernelTicks(void);
#else
int KernelInfo(int, void *);
int KernelTicks(void);
#endif

#endif // !_SYSTEM_H