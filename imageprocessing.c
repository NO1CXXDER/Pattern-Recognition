/*
 * @Name : imageprocessing.c
 * @Description : Image Processing in C (UNICODE / Windows)
 * @Date : 2026. 9. 19
 * @Revision : 0.2
 * @Author : Howoong Lee, Department of Computer Engineering, Hoseo Univ.
 * @History
 *  - 0.1 : main, process image, inverse, brightness, contrast
 *  - 0.2 : histogram(csv), binarization, gonzalez method, histogram stretching, histogram equalization
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <windows.h>
#include <commdlg.h>
#include <io.h>
#include <fcntl.h>
#include <math.h>

#include "msgstrings.h"

#ifdef _MSC_VER
#pragma comment(lib, "comdlg32.lib")    // GetOpenFileName / GetSaveFileName
#endif

#define PALETTE_SIZE 256                // 8bpp 팔레트 항목 수
#define GRAY_LEVELS  256                // 8bpp 밝기 레벨 수 (히스토그램 크기)

// 메뉴 번호 (msgstrings.h의 메뉴 문자열 순서와 일치해야 됨)
typedef enum {
    MODE_EXIT = 0,          // 0 : 프로그램 종료
    MODE_INVERSE,           // 1 : 영상 반전
    MODE_BRIGHTNESS,        // 2 : 밝기 조절
    MODE_CONTRAST,          // 3 : 대비 조절
    MODE_HISTOGRAM,         // 4 : 히스토그램 생성 (BMP 대신 CSV로 저장)
    MODE_GONZALEZ,          // 5 : 이진화 (Gonzalez Method)
    MODE_BINARIZATION,      // 6 : 이진화 (사용자 임계값)
    MODE_STRETCHING,        // 7 : 히스토그램 스트레칭
    MODE_EQUALIZATION,      // 8 : 히스토그램 평활화
    MODE_MAX = MODE_EQUALIZATION    // 메뉴 번호 최대값 : 메뉴 추가 시 마지막 항목으로 변경
} PROCESS_MODE;

 /*
  * @Function Name : ReadImgFileDialog
  * @Description   : 파일 열기 다이얼로그 박스
  * @Input         : *outPath, outCount
  * @Output        : TRUE(성공), FALSE(취소 또는 오류)
  */
static BOOL ReadImgFileDialog(TCHAR* outPath, DWORD outCount)
{
    OPENFILENAME ofn = { 0 };
    TCHAR szFile[MAX_PATH] = { 0 };

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetConsoleWindow();
    ofn.lpstrFilter = MSG_DLG_FILTER;      // 헤더 상수
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = MSG_DLG_TITLE_OPEN;  // 헤더 상수
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    // 파일 열기 대화상자 표시
    if (GetOpenFileName(&ofn)) {
        _tcsncpy_s(outPath, outCount, ofn.lpstrFile, _TRUNCATE);
        return TRUE;
    }
    else {
        // 오류 또는 취소 처리
        DWORD err = CommDlgExtendedError();
        if (err != 0) {
            _tprintf(MSG_ERROR_GETOPENFILENAME, err);
        }
        else {
            _tprintf(MSG_CANCEL_OPEN);
        }
        return FALSE;
    }
}

/*
 * @Function Name : SaveImgFileDialog
 * @Description   : 파일 저장 다이얼로그 박스
 * @Input         : *outPath, outCount
 * @Output        : TRUE(성공), FALSE(취소 또는 오류)
 */
static BOOL SaveImgFileDialog(TCHAR* outPath, DWORD outCount)
{
    OPENFILENAME ofn = { 0 };
    TCHAR szFile[MAX_PATH] = { 0 };

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetConsoleWindow();
    ofn.lpstrFilter = MSG_DLG_FILTER;      // 헤더 상수
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = MSG_DLG_TITLE_SAVE;  // 헤더 상수
    ofn.lpstrDefExt = _T("bmp");
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_PATHMUSTEXIST;

    // 파일 저장 대화상자 표시
    if (GetSaveFileName(&ofn)) {
        _tcsncpy_s(outPath, outCount, ofn.lpstrFile, _TRUNCATE);
        return TRUE;
    }
    else {
        // 오류 또는 취소 처리
        DWORD err = CommDlgExtendedError();
        if (err != 0) {
            _tprintf(MSG_ERROR_GETSAVEFILENAME, err);
        }
        else {
            _tprintf(MSG_CANCEL_SAVE);
        }
        return FALSE;
    }
}

/*
 * @Function Name : SaveCsvFileDialog
 * @Description   : 히스토그램 CSV 파일 저장 다이얼로그 박스
 * @Input         : *outPath, outCount
 * @Output        : TRUE(성공), FALSE(취소 또는 오류)
 */
static BOOL SaveCsvFileDialog(TCHAR* outPath, DWORD outCount)
{
    OPENFILENAME ofn = { 0 };
    TCHAR szFile[MAX_PATH] = { 0 };

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetConsoleWindow();
    ofn.lpstrFilter = MSG_DLG_FILTER_CSV;      // 헤더 상수
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = MSG_DLG_TITLE_SAVE_CSV;  // 헤더 상수
    ofn.lpstrDefExt = _T("csv");
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_PATHMUSTEXIST;

    // 파일 저장 대화상자 표시
    if (GetSaveFileName(&ofn)) {
        _tcsncpy_s(outPath, outCount, ofn.lpstrFile, _TRUNCATE);
        return TRUE;
    }
    else {
        // 오류 또는 취소 처리
        DWORD err = CommDlgExtendedError();
        if (err != 0) {
            _tprintf(MSG_ERROR_GETSAVEFILENAME, err);
        }
        else {
            _tprintf(MSG_CANCEL_SAVE);
        }
        return FALSE;
    }
}

/*
 * @Function Name   : InverseImage
 * @Description     : Pixel 단위로 밝기값을 Inverse
 * @Input           : *Input, nWidth, nHeight
 * @Output          : *Output
 */
static void InverseImage(const BYTE* Input, BYTE* Output, DWORD nWidth, DWORD nHeight)
{
    const SIZE_T nImgSize = (SIZE_T)nWidth * (SIZE_T)nHeight;

    // 픽셀 단위로 처리
    for (SIZE_T i = 0; i < nImgSize; i++)
        Output[i] = (BYTE)(255 - Input[i]);        // 255 - Pixel 값
}

/*
 * @Function Name : AdjustBrightness
 * @Description : nBrightness에 설정된 값을 Pixel 단위로 +. - 를 통한 밝기값을 조정
 * @Input : *Input, nWidth, nHeight, nBrightness
 * @Output : *Output
 */
static void AdjustBrightness(const BYTE* Input, BYTE* Output, DWORD nWidth, DWORD nHeight, LONG nBrightness)
{
    // nBrightness : 양수(밝게), 음수(어둡게)
    const SIZE_T nImgSize = (SIZE_T)nWidth * (SIZE_T)nHeight;

    // 픽셀 단위로 처리
    for (SIZE_T i = 0; i < nImgSize; i++) {

        // 밝기 조정 공식 적용
        int nVal = (int)Input[i] + (int)nBrightness;

        // 범위 체크
        if (nVal > 255)
            nVal = 255;
        else if (nVal < 0)
            nVal = 0;

        Output[i] = (BYTE)nVal;
    }
}

/*
 * @Function Name : AdjustContrast
 * @Description : dContrast에 설정된 값을 Pixel 단위로 *를 통한 대비값을 조정 (기준값 1 )
 * @Input : *Input, nWidth, nHeight, dContrast
 * @Output : *Output
 */
static void AdjustContrast(const BYTE* Input, BYTE* Output, DWORD nWidth, DWORD nHeight, DOUBLE dContrast)
{
    // dContrast : 1.0(원본), >1.0(대비 증가), <1.0(대비 감소)
    const SIZE_T nImgSize = (SIZE_T)nWidth * (SIZE_T)nHeight;

    // 픽셀 단위로 처리
    for (SIZE_T i = 0; i < nImgSize; i++) {

        // 대비 조정 공식 적용
        int nVal = (int)lround((DOUBLE)Input[i] * dContrast);  // 반올림 처리를 위해 lround 함수 적용

        // 범위 체크
        if (nVal > 255)
            nVal = 255;
        else if (nVal < 0)
            nVal = 0;

        Output[i] = (BYTE)nVal;
    }
}

/*
 * @Function Name : GenerateHistogram
 * @Description   : 입력 이미지에 대한 히스토그램을 버퍼에 출력
 * @Input         : *Input, nWidth, nHeight
 * @Output        : *Histogram (GRAY_LEVELS 크기의 배열)
 */
static void GenerateHistogram(const BYTE* Input, DWORD* Histogram, DWORD nWidth, DWORD nHeight)
{
    const SIZE_T nImgSize = (SIZE_T)nWidth * (SIZE_T)nHeight;

    // 누적(++) 방식이므로 호출한 쪽의 초기화 여부와 관계없이 먼저 0으로 초기화
    memset(Histogram, 0, sizeof(DWORD) * GRAY_LEVELS);

    // 히스토그램 생성
    for (SIZE_T i = 0; i < nImgSize; i++) {
        Histogram[Input[i]]++;
    }
}

/*
 * @Function Name : GenerateBinarization
 * @Description   : bThreshold 값을 임계값으로 하여 이진화를 진행
 * @Input         : *Input, nWidth, nHeight, bThreshold
 * @Output        : *Output
 */
static void GenerateBinarization(const BYTE* Input, BYTE* Output, DWORD nWidth, DWORD nHeight, BYTE bThreshold)
{
    const SIZE_T nImgSize = (SIZE_T)nWidth * (SIZE_T)nHeight;

    // 픽셀 단위로 처리
    for (SIZE_T i = 0; i < nImgSize; i++) {
        if (Input[i] >= bThreshold)
            Output[i] = 255;  // 임계값 이상은 흰색
        else
            Output[i] = 0;    // 임계값 미만은 검은색
    }
}

/*
 * @Function Name : GonzalezMethod
 * @Description   : Gonzalez, Woods Method에 따라 최적의 이진화 임계값을 계산
 * @Input         : *Histogram
 * @Output        : bThreshold
 */
static BYTE GonzalezMethod(const DWORD* Histogram)
{
    BYTE bLow = 0, bHigh = 0;
    BYTE bThreshold, bNewThreshold;
    const INT e = 2;                // 오차값 설정
    const INT nMaxIter = 256;       // 무한 반복 방지를 위한 최대 반복 횟수

    ULONGLONG nG1, nG2, nCntG1, nCntG2;
    DWORD nMeanG1, nMeanG2;

    // 영상에서 가장 어두운 값
    for (DWORD i = 0; i < GRAY_LEVELS; i++) {
        if (Histogram[i] != 0) {
            bLow = (BYTE)i;
            break;
        }
    }

    // 영상에서 가장 밝은 값
    for (LONG i = GRAY_LEVELS - 1; i >= 0; i--) {
        if (Histogram[i] != 0) {
            bHigh = (BYTE)i;
            break;
        }
    }

    // 1. Threshold의 초기값을 추정 : (최소값 + 최대값) / 2
    bThreshold = (BYTE)(((DWORD)bLow + (DWORD)bHigh) / 2);
    _tprintf(MSG_MENU_LINE);
    _tprintf(MSG_INIT_THRESHOLD, (UINT)bThreshold);

    // 단일 밝기 영상은 분할할 수 없으므로 초기값을 그대로 사용
    if (bLow == bHigh) {
        _tprintf(MSG_FINAL_THRESHOLD, (UINT)bThreshold);
        _tprintf(MSG_MENU_LINE);
        return bThreshold;
    }

    // 2~5번을 오차가 e보다 작을 때까지 반복 : e = 2로 설정
    for (INT nIter = 0; nIter < nMaxIter; nIter++) {

        // 반복마다 변수를 0으로 초기화
        nG1 = nG2 = nCntG1 = nCntG2 = 0;

        // 2. Threshold를 기준으로 영상을 분할
        // Threshold 이하의 값들을 G1에 추가 (Threshold는 G1에 포함)
        for (DWORD i = bLow; i <= bThreshold; i++) {
            nG1 += ((ULONGLONG)Histogram[i] * (ULONGLONG)i);
            nCntG1 += (ULONGLONG)Histogram[i];
        }

        // Threshold 보다 큰 값들을 G2에 추가
        for (DWORD i = (DWORD)bThreshold + 1; i <= bHigh; i++) {
            nG2 += ((ULONGLONG)Histogram[i] * (ULONGLONG)i);
            nCntG2 += (ULONGLONG)Histogram[i];
        }

        // 오류처리 : 0으로 나누기 오류
        if (0 == nCntG1) nCntG1 = 1;
        if (0 == nCntG2) nCntG2 = 1;

        // 3. G1, G2의 밝기 평균값 계산
        nMeanG1 = (DWORD)(nG1 / nCntG1);
        nMeanG2 = (DWORD)(nG2 / nCntG2);

        // 4. 새로운 임계값을 계산
        bNewThreshold = (BYTE)((nMeanG1 + nMeanG2) / 2);

        // 5. 오차가 e보다 작은지 검사
        if (abs((INT)bNewThreshold - (INT)bThreshold) < e) {
            // 오차 범위 내 Threshold 최종 결정
            bThreshold = bNewThreshold;
            break;
        }

        bThreshold = bNewThreshold;
        _tprintf(MSG_NEW_THRESHOLD, (UINT)bNewThreshold);
    }

    _tprintf(MSG_FINAL_THRESHOLD, (UINT)bThreshold);
    _tprintf(MSG_MENU_LINE);

    return bThreshold;
}

/*
 * @Function Name : HistogramStretching
 * @Description   : 히스토그램 스트레칭을 수행
 * @Input         : *Input, *Histogram, nWidth, nHeight
 * @Output        : *Output
 */
static void HistogramStretching(const BYTE* Input, BYTE* Output, const DWORD* Histogram, DWORD nWidth, DWORD nHeight)
{
    const SIZE_T nImgSize = (SIZE_T)nWidth * (SIZE_T)nHeight;
    BYTE bLow = 0, bHigh = 0;

    // 히스토그램에서 최초로 0이 아닌 밝기 값을 계산
    for (DWORD i = 0; i < GRAY_LEVELS; i++) {
        if (Histogram[i] != 0) {
            bLow = (BYTE)i;
            break;
        }
    }

    // 히스토그램에서 마지막으로 0이 아닌 밝기 값을 계산
    for (LONG i = GRAY_LEVELS - 1; i >= 0; i--) {
        if (Histogram[i] != 0) {
            bHigh = (BYTE)i;
            break;
        }
    }

    // 단일 밝기 영상은 High - Low = 0 이므로 스트레칭할 수 없음 : 원본 유지
    if (bHigh == bLow) {
        memcpy(Output, Input, nImgSize);
        return;
    }

    // 스트레칭 수행
    // (Input[i] - Low)  : 밝기의 최소값이 0이 되도록 이동
    // / (High - Low)    : 최대 밝기 값과 최소 밝기 값의 차이로 나누어 0 ~ 1로 정규화
    // X 255             : 밝기 값을 0 ~ 255 범위로 스케일링
    const DOUBLE dScale = 255.0 / (DOUBLE)(bHigh - bLow);

    for (SIZE_T i = 0; i < nImgSize; i++) {
        DOUBLE dStretch = ((DOUBLE)Input[i] - (DOUBLE)bLow) * dScale;
        Output[i] = (BYTE)lround(dStretch);     // 반올림 처리
    }
}

/*
 * @Function Name : HistogramEqualization
 * @Description   : 히스토그램 평활화를 수행
 * @Input         : *Input, *Histogram, nWidth, nHeight
 * @Output        : *Output
 */
static void HistogramEqualization(const BYTE* Input, BYTE* Output, const DWORD* Histogram, DWORD nWidth, DWORD nHeight)
{
    const SIZE_T nImgSize = (SIZE_T)nWidth * (SIZE_T)nHeight;

    const DWORD Nt = (DWORD)nImgSize;           // 총 픽셀 수 (ReadBmpData에서 DWORD 범위로 제한됨)
    const DWORD Gmax = GRAY_LEVELS - 1;         // 이미지에서 최대 밝기 레벨 (255)

    const DOUBLE Ratio = (DOUBLE)Gmax / (DOUBLE)Nt;    // 최대 밝기 레벨을 전체 픽셀 수로 나눈 비율

    BYTE NormSum[GRAY_LEVELS] = { 0, };         // 정규화된 누적 히스토그램 (변환 테이블)
    ULONGLONG nSum = 0;                         // 누적 히스토그램 값

    // 누적 히스토그램을 구하면서 Ratio를 곱해서 0 ~ 255 범위로 정규화
    // 마지막 누적값은 Nt이므로 Nt X (Gmax / Nt) = Gmax = 255
    for (DWORD i = 0; i < GRAY_LEVELS; i++) {
        nSum += Histogram[i];
        NormSum[i] = (BYTE)lround(Ratio * (DOUBLE)nSum);   // 반올림 처리
    }

    // Input의 각 픽셀값에 대응하는 정규화된 누적 히스토그램 값을 Output에 저장
    for (SIZE_T i = 0; i < nImgSize; i++) {
        Output[i] = NormSum[Input[i]];
    }
}


/*
 * @Function Name : ReadBmpData
 * @Description   : 열려 있는 BMP 파일에서 헤더, 팔레트, 픽셀 데이터를 읽음
 *                  파일을 열고 닫는 것은 호출한 함수(LoadBmpFile)에서 처리
 * @Input         : fp
 * @Output        : *phf, *phInfo, hRGB, *ppInput(성공 시 malloc된 픽셀 버퍼 : 호출한 함수에서 free)
 *                  TRUE(성공), FALSE(오류)
 */
static BOOL ReadBmpData(FILE* fp, BITMAPFILEHEADER* phf, BITMAPINFOHEADER* phInfo, RGBQUAD* hRGB, BYTE** ppInput)
{
    // 헤더 읽기
    if (fread(phf, sizeof(BITMAPFILEHEADER), 1, fp) != 1 ||
        fread(phInfo, sizeof(BITMAPINFOHEADER), 1, fp) != 1) {
        _tprintf(MSG_ERROR_READ_HEADER);
        return FALSE;
    }

    // 8bpp 비압축 BMP만 허용
    if (phf->bfType != 0x4D42 || phInfo->biSize < sizeof(BITMAPINFOHEADER) ||
        phInfo->biBitCount != 8 || phInfo->biCompression != BI_RGB) {
        _tprintf(MSG_ERROR_BMP);
        return FALSE;
    }

    // 상하 반전 이미지(음수 높이) 불허
    if (phInfo->biHeight < 0) {
        _tprintf(MSG_ERROR_IMAGE_TOPDOWN);
        return FALSE;
    }

    // 잘못된 크기(0 이하) 불허
    if (phInfo->biHeight == 0 || phInfo->biWidth <= 0) {
        _tprintf(MSG_ERROR_IMAGE_SIZE);
        return FALSE;
    }

    // 가로 픽셀 수가 4의 배수(Stride 4바이트 정렬)인지 확인
    if ((phInfo->biWidth & 3) != 0) {
        _tprintf(MSG_ERROR_IMAGE_WIDTH);
        return FALSE;
    }

    // nWidth * nHeight 오버플로 방지 (biSizeImage는 DWORD)
    const DWORD nWidth = (DWORD)phInfo->biWidth;
    const DWORD nHeight = (DWORD)phInfo->biHeight;
    if (nWidth > 0xFFFFFFFFu / nHeight) {
        _tprintf(MSG_ERROR_IMAGE_SIZE);
        return FALSE;
    }
    const SIZE_T nImgSize = (SIZE_T)nWidth * (SIZE_T)nHeight;

    // 팔레트 읽기
    // 팔레트는 정보 헤더(biSize 바이트) 바로 뒤에 위치 (V4/V5 헤더 대응)
    // biClrUsed가 0이면 256개, 아니면 biClrUsed개만 저장되어 있음
    const DWORD nColors = (phInfo->biClrUsed == 0) ? PALETTE_SIZE : phInfo->biClrUsed;
    if (nColors > PALETTE_SIZE) {
        _tprintf(MSG_ERROR_BMP);
        return FALSE;
    }
    if (_fseeki64(fp, (__int64)sizeof(BITMAPFILEHEADER) + phInfo->biSize, SEEK_SET) != 0 ||
        fread(hRGB, sizeof(RGBQUAD), nColors, fp) != nColors) {
        _tprintf(MSG_ERROR_READ_PALETTE);
        return FALSE;
    }

    // 픽셀 데이터 위치로 이동
    if (_fseeki64(fp, phf->bfOffBits, SEEK_SET) != 0) {
        _tprintf(MSG_ERROR_SEEK);
        return FALSE;
    }

    // 픽셀 데이터 메모리 할당 : 여기부터는 실패 시 메모리를 해제해야 하므로 마지막 단계에 배치
    BYTE* Input = (BYTE*)malloc(nImgSize);
    if (!Input) {
        _tprintf(MSG_ERROR_MEM_ALLOCATION);
        return FALSE;
    }

    // 픽셀 데이터 읽기
    if (fread(Input, 1, nImgSize, fp) != nImgSize) {
        _tprintf(MSG_ERROR_READ_PIXEL);
        free(Input);
        return FALSE;
    }

    *ppInput = Input;       // 성공 : 버퍼의 소유권을 호출한 쪽으로 넘김
    return TRUE;
}


/*
 * @Function Name : LoadBmpFile
 * @Description   : BMP 파일을 열어 ReadBmpData로 읽고 닫음 (파일 열기/닫기를 한 곳에서 처리)
 * @Input         : *inPath
 * @Output        : *phf, *phInfo, hRGB, *ppInput(성공 시 malloc된 픽셀 버퍼 : 호출한 함수에서 free)
 *                  TRUE(성공), FALSE(오류)
 */
static BOOL LoadBmpFile(const TCHAR* inPath, BITMAPFILEHEADER* phf, BITMAPINFOHEADER* phInfo, RGBQUAD* hRGB, BYTE** ppInput)
{
    FILE* fp = NULL;
    errno_t nErr = _tfopen_s(&fp, inPath, _T("rb"));
    if (!fp) {
        _tprintf(MSG_ERROR_FILE, nErr, inPath);
        return FALSE;
    }

    BOOL bResult = ReadBmpData(fp, phf, phInfo, hRGB, ppInput);

    fclose(fp);             // 성공/실패와 관계없이 이곳에서만 닫음

    return bResult;
}


/*
 * @Function Name : WriteBmpData
 * @Description   : 열려 있는 파일에 헤더, 팔레트, 픽셀 데이터를 기록
 *                  파일을 열고 닫는 것은 호출한 함수(SaveBmpFile)에서 처리
 * @Input         : fp, *phf, *phInfo, hRGB, *Output
 * @Output        : TRUE(성공), FALSE(오류)
 */
static BOOL WriteBmpData(FILE* fp, const BITMAPFILEHEADER* phf, const BITMAPINFOHEADER* phInfo, const RGBQUAD* hRGB, const BYTE* Output)
{
    const SIZE_T nImgSize = (SIZE_T)phInfo->biWidth * (SIZE_T)phInfo->biHeight;

    // 처리 결과에 따른 헤더 정보 갱신 
    // 출력 파일은 항상 "파일 헤더 + BITMAPINFOHEADER + 256색 팔레트 + 픽셀" 구조로 저장
    // 원본 값과 무관하게 biSize, biClrUsed, bfOffBits를 실제 저장 구조에 맞게 설정
    BITMAPFILEHEADER hf = *phf;
    BITMAPINFOHEADER hInfo = *phInfo;

    hInfo.biSize = sizeof(BITMAPINFOHEADER);
    hInfo.biClrUsed = 0;                                // 0 = 팔레트 256개 모두 사용
    hInfo.biClrImportant = 0;
    hInfo.biSizeImage = (DWORD)nImgSize;                // 실제 픽셀 데이터 크기
    hf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * PALETTE_SIZE;
    hf.bfSize = hf.bfOffBits + hInfo.biSizeImage;       // 파일 전체 크기

    // 헤더 쓰기
    if (fwrite(&hf, sizeof(BITMAPFILEHEADER), 1, fp) != 1 ||
        fwrite(&hInfo, sizeof(BITMAPINFOHEADER), 1, fp) != 1) {
        _tprintf(MSG_ERROR_WRITE_HEADER);
        return FALSE;
    }

    // 팔레트 쓰기
    if (fwrite(hRGB, sizeof(RGBQUAD), PALETTE_SIZE, fp) != PALETTE_SIZE) {
        _tprintf(MSG_ERROR_WRITE_PALETTE);
        return FALSE;
    }

    // 픽셀 데이터 쓰기
    if (fwrite(Output, 1, nImgSize, fp) != nImgSize) {
        _tprintf(MSG_ERROR_WRITE_PIXEL);
        return FALSE;
    }

    return TRUE;
}


/*
 * @Function Name : SaveBmpFile
 * @Description   : 출력 파일을 열어 WriteBmpData로 기록하고 닫음 (파일 열기/닫기를 한 곳에서 처리)
 * @Input         : *outPath, *phf, *phInfo, hRGB, *Output
 * @Output        : TRUE(성공), FALSE(오류)
 */
static BOOL SaveBmpFile(const TCHAR* outPath, const BITMAPFILEHEADER* phf, const BITMAPINFOHEADER* phInfo, const RGBQUAD* hRGB, const BYTE* Output)
{
    FILE* fp = NULL;
    errno_t nErr = _tfopen_s(&fp, outPath, _T("wb"));
    if (!fp) {
        _tprintf(MSG_ERROR_FILE, nErr, outPath);
        return FALSE;
    }

    BOOL bResult = WriteBmpData(fp, phf, phInfo, hRGB, Output);

    // 성공/실패와 관계없이 이곳에서만 닫음
    // 버퍼에 남은 데이터의 실제 기록은 fclose에서 일어나므로 결과도 확인
    if (fclose(fp) != 0 && bResult) {
        _tprintf(MSG_ERROR_WRITE_PIXEL);
        bResult = FALSE;
    }

    return bResult;
}


/*
 * @Function Name : WriteHistogramCsv
 * @Description   : 열려 있는 파일에 히스토그램을 CSV 형식(밝기, 픽셀 수)으로 기록
 *                  파일을 열고 닫는 것은 호출한 함수(SaveHistogramCsv)에서 처리
 * @Input         : fp, *Histogram
 * @Output        : TRUE(성공), FALSE(오류)
 */
static BOOL WriteHistogramCsv(FILE* fp, const DWORD* Histogram)
{
    // 헤더 행 쓰기
    if (_ftprintf(fp, MSG_CSV_HEADER) < 0) {
        _tprintf(MSG_ERROR_WRITE_CSV);
        return FALSE;
    }

    // 밝기 값(0 ~ 255)별 픽셀 수 쓰기
    for (DWORD i = 0; i < GRAY_LEVELS; i++) {
        if (_ftprintf(fp, MSG_CSV_ROW, (UINT)i, Histogram[i]) < 0) {
            _tprintf(MSG_ERROR_WRITE_CSV);
            return FALSE;
        }
    }

    return TRUE;
}


/*
 * @Function Name : SaveHistogramCsv
 * @Description   : CSV 파일을 열어 WriteHistogramCsv로 기록하고 닫음 (파일 열기/닫기를 한 곳에서 처리)
 * @Input         : *outPath, *Histogram
 * @Output        : TRUE(성공), FALSE(오류)
 */
static BOOL SaveHistogramCsv(const TCHAR* outPath, const DWORD* Histogram)
{
    FILE* fp = NULL;
    errno_t nErr = _tfopen_s(&fp, outPath, _T("w"));    // 텍스트 모드 : 줄바꿈을 CRLF로 저장
    if (!fp) {
        _tprintf(MSG_ERROR_FILE, nErr, outPath);
        return FALSE;
    }

    BOOL bResult = WriteHistogramCsv(fp, Histogram);

    // 성공/실패와 관계없이 이곳에서만 닫음 (버퍼 기록 결과도 확인)
    if (fclose(fp) != 0 && bResult) {
        _tprintf(MSG_ERROR_WRITE_CSV);
        bResult = FALSE;
    }

    return bResult;
}


/*
 * @Function Name : ProcessImage
 * @Description   : nMode에 따라 필요한 값을 입력받고 해당 영상처리 함수를 호출
 *                  MODE_HISTOGRAM은 영상 대신 히스토그램을 *Histogram에 생성 (CSV 저장은 main에서 처리)
 * @Input         : nMode, *Input, nWidth, nHeight
 * @Output        : *Output, *Histogram, TRUE(성공), FALSE(입력 오류)
 */
static BOOL ProcessImage(PROCESS_MODE nMode, const BYTE* Input, BYTE* Output, DWORD* Histogram, DWORD nWidth, DWORD nHeight)
{
    /*------- 기능별 변수 선언 ----------------------------------------------------*/
    LONG   nBrightness = 0;
    DOUBLE dContrast = 0.0;
    UINT   nThreshold = 0;                          // 사용자 입력 임계값 (범위 검사 후 BYTE로 변환)
    BYTE   bThreshold = 0;

    /*------- 선택된 기능 수행 ----------------------------------------------------*/
    switch (nMode) {

    case MODE_INVERSE:
        InverseImage(Input, Output, nWidth, nHeight);

        break;

    case MODE_BRIGHTNESS:
        _tprintf(MSG_PROMPT_BRIGHTNESS);
        if (_tscanf_s(_T("%ld"), &nBrightness) != 1) {
            _tprintf(MSG_ERROR_INPUT);
            return FALSE;
        }

        AdjustBrightness(Input, Output, nWidth, nHeight, nBrightness);

        break;

    case MODE_CONTRAST:
        _tprintf(MSG_PROMPT_CONTRAST);
        if (_tscanf_s(_T("%lf"), &dContrast) != 1 || dContrast <= 0.0) {
            _tprintf(MSG_ERROR_INPUT);
            return FALSE;
        }

        AdjustContrast(Input, Output, nWidth, nHeight, dContrast);

        break;

    case MODE_HISTOGRAM:
        // 히스토그램 생성 (CSV 저장은 main에서 처리)
        GenerateHistogram(Input, Histogram, nWidth, nHeight);

        break;

    case MODE_GONZALEZ:
        // 히스토그램 생성 → Gonzalez Method로 임계값 계산 → 이진화 수행
        GenerateHistogram(Input, Histogram, nWidth, nHeight);
        bThreshold = GonzalezMethod(Histogram);
        GenerateBinarization(Input, Output, nWidth, nHeight, bThreshold);

        break;

    case MODE_BINARIZATION:
        // 임계값 입력 → 이진화 수행
        _tprintf(MSG_PROMPT_THRESHOLD);
        if (_tscanf_s(_T("%u"), &nThreshold) != 1 || nThreshold > 255) {
            _tprintf(MSG_ERROR_INPUT);
            return FALSE;
        }

        GenerateBinarization(Input, Output, nWidth, nHeight, (BYTE)nThreshold);

        break;

    case MODE_STRETCHING:
        // 히스토그램 생성 → 히스토그램 스트레칭 수행
        GenerateHistogram(Input, Histogram, nWidth, nHeight);
        HistogramStretching(Input, Output, Histogram, nWidth, nHeight);

        break;

    case MODE_EQUALIZATION:
        // 히스토그램 생성 → 히스토그램 평활화 수행
        GenerateHistogram(Input, Histogram, nWidth, nHeight);
        HistogramEqualization(Input, Output, Histogram, nWidth, nHeight);

        break;

    case MODE_EXIT:             // 종료(0)는 main에서 먼저 처리하므로 여기로 오지 않음
    default:
        _tprintf(MSG_ERROR_INPUT);
        return FALSE;
    }

    return TRUE;
}


/*
 * @Function Name : main
 * @Description : Image Processing main 함수로 메뉴 선택 → 읽기 → 처리 → 저장 순서로 함수를 호출
 * @Input : 없음
 * @Output : ERROR_SUCCESS(성공), -1(오류)
 */
int _tmain(void)
{
    // 콘솔을 UTF-16 모드로 변환 : 한글 출력용
    if (_isatty(_fileno(stdout))) _setmode(_fileno(stdout), _O_U16TEXT);
    if (_isatty(_fileno(stdin)))  _setmode(_fileno(stdin), _O_U16TEXT);
    if (_isatty(_fileno(stderr))) _setmode(_fileno(stderr), _O_U16TEXT);

    // 프로그램 타이틀 및 메뉴 출력
    _tprintf(MSG_MENU_BEGIN);
    _tprintf(MSG_APP_TITLE);
    _tprintf(_T("\n\n"));
    _tprintf(MSG_MENU_INVERSE);
    _tprintf(MSG_MENU_BRIGHTNESS);
    _tprintf(MSG_MENU_CONTRAST);
    _tprintf(MSG_MENU_HISTOGRAM);
    _tprintf(MSG_MENU_GONZALEZ);
    _tprintf(MSG_MENU_BINARIZATION);
    _tprintf(MSG_MENU_STRETCHING);
    _tprintf(MSG_MENU_EQUALIZATION);

    _tprintf(MSG_MENU_EXIT);
    _tprintf(MSG_MENU_END);

    /*------- 자원을 할당하기 전 단계 : 해제할 것이 없으므로 바로 return ------------*/

    // 기능 선택
    UINT nMode = 0;
    _tprintf(MSG_PROMPT_MODE);
    if (_tscanf_s(_T("%u"), &nMode) != 1 || nMode > MODE_MAX) {
        _tprintf(MSG_ERROR_INPUT);
        return -1;
    }

    // 0이면 바로 종료: 이후 어떤 작업도 하지 않음
    if (nMode == MODE_EXIT) {
        _tprintf(MSG_INFO_EXIT);
        return ERROR_SUCCESS;
    }

    // 입력 파일 선택
    TCHAR inPath[MAX_PATH] = { 0 };
    if (!ReadImgFileDialog(inPath, MAX_PATH)) return -1;

    /*------- 자원(메모리)을 사용하는 단계 : 해제는 함수 끝 한 곳에서만 수행 ---------*/
    int               nResult = -1;         // 끝까지 성공한 경우에만 ERROR_SUCCESS로 변경
    BYTE* Input = NULL;         // free(NULL)은 안전하므로 반드시 NULL로 초기화
    BYTE* Output = NULL;
    BITMAPFILEHEADER  hf;
    BITMAPINFOHEADER  hInfo;
    RGBQUAD           hRGB[PALETTE_SIZE] = { 0 };
    TCHAR             outPath[MAX_PATH] = { 0 };
    DWORD             Histogram[GRAY_LEVELS] = { 0 };   // 히스토그램 버퍼 (스택 배열 : 해제 불필요)

    // 입력 파일 읽기 (파일 열기/닫기는 LoadBmpFile 내부에서 처리)
    if (LoadBmpFile(inPath, &hf, &hInfo, hRGB, &Input)) {

        const DWORD  nWidth = (DWORD)hInfo.biWidth;
        const DWORD  nHeight = (DWORD)hInfo.biHeight;
        const SIZE_T nImgSize = (SIZE_T)nWidth * (SIZE_T)nHeight;

        _tprintf(MSG_INFO_SIZE, nWidth, nHeight, (UINT)hInfo.biBitCount);

        // 출력 영상 메모리 할당
        Output = (BYTE*)malloc(nImgSize);

        if (!Output) {
            _tprintf(MSG_ERROR_MEM_ALLOCATION);
        }
        // 영상처리 → 출력 파일 선택 → 저장 : 앞 단계가 성공해야 다음 단계를 수행 (&&의 단축 평가)
        else if (ProcessImage((PROCESS_MODE)nMode, Input, Output, Histogram, nWidth, nHeight)) {

            BOOL bSaved;
            if (nMode == MODE_HISTOGRAM) {
                // 히스토그램 : CSV 파일로 저장
                bSaved = SaveCsvFileDialog(outPath, MAX_PATH) &&
                    SaveHistogramCsv(outPath, Histogram);
            }
            else {
                // 영상 : BMP 파일로 저장
                bSaved = SaveImgFileDialog(outPath, MAX_PATH) &&
                    SaveBmpFile(outPath, &hf, &hInfo, hRGB, Output);
            }

            if (bSaved) {
                _tprintf(MSG_INFO_DONE, outPath);
                nResult = ERROR_SUCCESS;
            }
        }
    }

    /*------- 종료 처리 : 성공/실패 모두 이곳에서 메모리 해제 -----------------------*/
    free(Input);
    free(Output);

    return nResult;
}