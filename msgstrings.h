/*
 * @Name : msgstrings.h
 * @Description : Messages Header for Image Processing
 * @Date : 2026. 9. 19
 * @Revision : 1.1
 * @Author : Howoong Lee, Division of Computer Engineering, Hoseo Univ.
 * @History
 *  - 1.0 : title, menu, prompt, filter, error, info messages
 *  - 1.1 : histogram(csv), binarization, histogram stretching/equalization menu, threshold/csv messages
 */


#ifndef MSGSTRINGS_H
#define MSGSTRINGS_H

#include <tchar.h>

 // 프로그램 타이틀 및 메뉴
static const TCHAR* MSG_APP_TITLE = _T("Image Processing Program (UNICODE)\n");
static const TCHAR* MSG_MENU_BEGIN = _T("=================================\n\n");
static const TCHAR* MSG_MENU_INVERSE = _T("1.  Inverse Image\n");
static const TCHAR* MSG_MENU_BRIGHTNESS = _T("2.  Adjust Brightness\n");
static const TCHAR* MSG_MENU_CONTRAST = _T("3.  Adjust Contrast\n");
static const TCHAR* MSG_MENU_HISTOGRAM = _T("4.  Generate Histogram (CSV)\n");
static const TCHAR* MSG_MENU_GONZALEZ = _T("5.  Binarization (Gonzalez Method)\n");
static const TCHAR* MSG_MENU_BINARIZATION = _T("6.  Binarization (User Threshold)\n");
static const TCHAR* MSG_MENU_STRETCHING = _T("7.  Histogram Stretching\n");
static const TCHAR* MSG_MENU_EQUALIZATION = _T("8.  Histogram Equalization\n");

static const TCHAR* MSG_MENU_EXIT = _T("0.  Exit Program\n");
static const TCHAR* MSG_MENU_END = _T("\n=================================\n\n");
static const TCHAR* MSG_MENU_LINE = _T("---------------------------------\n");

// 사용자 입력 프롬프트
static const TCHAR* MSG_PROMPT_MODE = _T("원하는 기능의 번호를 입력하세요 : ");
static const TCHAR* MSG_PROMPT_BRIGHTNESS = _T("밝기 조절 값(정수)을 입력하세요 : ");
static const TCHAR* MSG_PROMPT_CONTRAST = _T("대비 조절 값(0보다 큰 실수 값)을 입력하세요 : ");
static const TCHAR* MSG_PROMPT_THRESHOLD = _T("이진화 임계값(0 ~ 255)을 입력하세요 : ");

// 파일 대화상자 제목/필터
// 필터 문자열은 항목별로 '\0' 로 구분, 마지막은 '\0\0'로 구분되어야 하므로 배열로 선언
static const TCHAR  MSG_DLG_FILTER[] = _T("Bitmap Files (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0");
static const TCHAR* MSG_DLG_TITLE_OPEN = _T("Open Image File");
static const TCHAR* MSG_DLG_TITLE_SAVE = _T("Save Image File");
static const TCHAR  MSG_DLG_FILTER_CSV[] = _T("CSV Files (*.csv)\0*.csv\0All Files (*.*)\0*.*\0");
static const TCHAR* MSG_DLG_TITLE_SAVE_CSV = _T("Save Histogram File (CSV)");

// 오류/취소 메시지
static const TCHAR* MSG_ERROR_IMAGE_TOPDOWN = _T("Error: top-down BMP (negative height) is not supported.\n");
static const TCHAR* MSG_ERROR_IMAGE_WIDTH = _T("Error: image width must be a multiple of 4.\n");
static const TCHAR* MSG_ERROR_IMAGE_SIZE = _T("Error: invalid image size.\n");
static const TCHAR* MSG_CANCEL_OPEN = _T("File selection was canceled.\n");
static const TCHAR* MSG_CANCEL_SAVE = _T("File save was canceled.\n");

static const TCHAR* MSG_ERROR_GETOPENFILENAME = _T("GetOpenFileName failed. CommDlgExtendedError = %lu\n");
static const TCHAR* MSG_ERROR_GETSAVEFILENAME = _T("GetSaveFileName failed. CommDlgExtendedError = %lu\n");

static const TCHAR* MSG_ERROR_FILE = _T("Failed to open file (error=%d): %s\n");
static const TCHAR* MSG_ERROR_READ_HEADER = _T("Error: failed to read BMP headers.\n");
static const TCHAR* MSG_ERROR_BMP = _T("Error: this program expects 8bpp uncompressed BMP.\n");
static const TCHAR* MSG_ERROR_READ_PALETTE = _T("Error: failed to read 8bpp palette.\n");
static const TCHAR* MSG_ERROR_SEEK = _T("Error: failed to seek to pixel data.\n");
static const TCHAR* MSG_ERROR_READ_PIXEL = _T("Error: failed to read pixel data.\n");
static const TCHAR* MSG_ERROR_WRITE_HEADER = _T("Error: failed to write BMP headers.\n");
static const TCHAR* MSG_ERROR_WRITE_PALETTE = _T("Error: failed to write 8bpp palette.\n");
static const TCHAR* MSG_ERROR_WRITE_PIXEL = _T("Error: failed to write pixel data.\n");
static const TCHAR* MSG_ERROR_WRITE_CSV = _T("Error: failed to write histogram csv file.\n");
static const TCHAR* MSG_ERROR_MEM_ALLOCATION = _T("Error: memory allocation error!\n");
static const TCHAR* MSG_ERROR_INPUT = _T("Error: invalid input value.\n");

// 정보 및 완료 메시지
static const TCHAR* MSG_INFO_SIZE = _T("Width = %u, Height = %u, BitCount = %u\n");
static const TCHAR* MSG_INFO_DONE = _T("Success: %s\n");
static const TCHAR* MSG_INFO_EXIT = _T("프로그램을 종료합니다.\n");

// 이진화 임계값 정보 메시지 (Gonzalez Method)
static const TCHAR* MSG_INIT_THRESHOLD = _T("Initial Threshold = %u\n");
static const TCHAR* MSG_NEW_THRESHOLD = _T("New Threshold = %u\n");
static const TCHAR* MSG_FINAL_THRESHOLD = _T("Final Threshold = %u\n");

// 히스토그램 CSV 파일 형식 (밝기, 픽셀 수)
static const TCHAR* MSG_CSV_HEADER = _T("Brightness,Count\n");
static const TCHAR* MSG_CSV_ROW = _T("%u,%lu\n");

#endif // MSGSTRINGS_H