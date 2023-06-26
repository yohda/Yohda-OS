/**
 *  file    ConsoleShell.h
 *  date    2009/01/31
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   콘솔 셸에 관련된 헤더 파일
 */

#ifndef __CONSOLESHELL_H__
#define __CONSOLESHELL_H__

#include "Types.h"
#include "test.h"
////////////////////////////////////////////////////////////////////////////////
//
// 매크로
//
////////////////////////////////////////////////////////////////////////////////
#define CONSOLESHELL_MAXCOMMANDBUFFERCOUNT  300
#define CONSOLESHELL_PROMPTMESSAGE          "MINT64>"

// 문자열 포인터를 파라미터로 받는 함수 포인터 타입 정의
typedef void ( * CommandFunction ) ( const char* pcParameter );


////////////////////////////////////////////////////////////////////////////////
//
// 구조체
//
////////////////////////////////////////////////////////////////////////////////
// 1바이트로 정렬
#pragma pack( push, 1 )

// 셸의 커맨드를 저장하는 자료구조
typedef struct kShellCommandEntryStruct
{
    // 커맨드 문자열
    char* pcCommand;
    // 커맨드의 도움말
    char* pcHelp;
    // 커맨드를 수행하는 함수의 포인터
    CommandFunction pfFunction;
} SHELLCOMMANDENTRY;

// 파라미터를 처리하기위해 정보를 저장하는 자료구조
typedef struct kParameterListStruct
{
    // 파라미터 버퍼의 어드레스
    const char* pcBuffer;
    // 파라미터의 길이
    int iLength;
    // 현재 처리할 파라미터가 시작하는 위치
    int iCurrentPosition;
} PARAMETERLIST;

#pragma pack( pop )

////////////////////////////////////////////////////////////////////////////////
//
// 함수
//
////////////////////////////////////////////////////////////////////////////////
// 실제 셸 코드
void kStartConsoleShell( void );
void kExecuteCommand( const char* pcCommandBuffer );
void kInitializeParameter( PARAMETERLIST* pstList, const char* pcParameter );
int kGetNextParameter( PARAMETERLIST* pstList, char* pcParameter );

// 커맨드를 처리하는 함수
void kHelp( const char* pcParameterBuffer );
void kCls( const char* pcParameterBuffer );
void kShowTotalRAMSize( const char* pcParameterBuffer );
void kStringToDecimalHexTest( const char* pcParameterBuffer );
void kShutdown( const char* pcParamegerBuffer );
void kSetTimer( const char* pcParameterBuffer );
void kWaitUsingPIT( const char* pcParameterBuffer );
void kReadTimeStampCounter( const char* pcParameterBuffer );
void kMeasureProcessorSpeed( const char* pcParameterBuffer );
void kShowDateAndTime( const char* pcParameterBuffer );

// yohdaOS
void mkdir(const char* pcParameterBuffer);
void sync(const char* pcParameterBuffer);
void touch(const char* pcParameterBuffer);
void pwd(const char* pcParameterBuffer);
void cd(const char* pcParameterBuffer);
void mv(const char* pcParameterBuffer);
void mem(const char* pcParameterBuffer);

// ls를 만드는 기준은 현재 내가 어떤 폴더를 바라보고 있냐와 관련이 있다.
// 이건 소프트웨어적으로 현재 경로를 관리하는 변수가 하나 필요할 듯 하다.
// yohdaOS는 Root Filesystem 아키텍처를 따르므로, 초기 부팅시에는 `/` 를 바라보게 한다.
void ls(const char* pcParameterBuffer);

// For test 
#ifdef BST_TEST
void bst(const char* pcParameterBuffer);
#endif

#ifdef STK_TEST
void stack(const char* pcParameterBuffer);
#endif

#ifdef STRING_TEST
void string(const char* pcParameterBuffer);
void _strrchr(const char* pcParameterBuffer);
#endif

#ifdef LIST_TEST
void _list(const char* pcParameterBuffer);
#endif

#ifdef PL_TEST
void _pool(const char* pcParameterBuffer);
#endif

#ifdef BUD_TEST
void _bud(const char* pcParameterBuffer);
#endif

#ifdef MMI_TEST
void _mmi(const char* pcParameterBuffer);
#endif

#ifdef LAZY_BUD_TEST
void _lazy_bud(const char* pcParameterBuffer);
#endif

#endif /*__CONSOLESHELL_H__*/
