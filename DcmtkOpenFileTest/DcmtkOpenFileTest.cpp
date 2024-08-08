#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmimgle/dcmimage.h"
#include "dcmtk/dcmjpeg/djdecode.h"
#include "dcmtk/dcmdata/dcrledrg.h"
#include <iostream>
#include <fstream>
#include <iomanip>

bool isDicomFile(const char* filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "파일을 열 수 없습니다: " << filename << std::endl;
        return false;
    }

    char buffer[132];
    file.read(buffer, 132);
    if (file.gcount() < 132) {
        std::cerr << "파일이 DICOM 파일로 보기에는 너무 작습니다." << std::endl;
        return false;
    }

    return (strncmp(buffer + 128, "DICM", 4) == 0);
}

void printFileHeader(const char* filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "파일을 열 수 없습니다: " << filename << std::endl;
        return;
    }

    char buffer[256];
    file.read(buffer, 256);
    std::cout << "파일 헤더 (처음 256 바이트):" << std::endl;
    for (int i = 0; i < 256; i++) {
        std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)(unsigned char)buffer[i] << " ";
        if ((i + 1) % 16 == 0) std::cout << std::endl;
    }
    std::cout << std::dec << std::endl;
}

int main(int argc, char* argv[]) {
    OFLog::configure(OFLogger::DEBUG_LOG_LEVEL);

    dcmIgnoreParsingErrors.set(OFTrue);
    dcmPreferVRFromDataDictionary.set(OFTrue);
    dcmIgnoreFileMetaInformationGroupLength.set(OFTrue);

    // JPEG 압축 해제를 위한 초기화
    DJDecoderRegistration::registerCodecs();
    // RLE 압축 해제를 위한 초기화 (필요한 경우)
    DcmRLEDecoderRegistration::registerCodecs();

    const char* filename = "D:\\dicom\\myAngio.dcm"; // 읽을 DICOM 파일 경로

    if (!isDicomFile(filename)) {
        std::cerr << "이 파일은 유효한 DICOM 파일이 아닙니다." << std::endl;
        printFileHeader(filename);
        return 1;
    }

    std::cout << "유효한 DICOM 파일" << std::endl;

    // 데이터 사전 로드 확인
    if (!dcmDataDict.isDictionaryLoaded()) {
        std::cout << "경고: 데이터 사전이 로드되지 않았습니다. 환경 변수를 확인하세요: "
            << DCM_DICT_ENVIRONMENT_VARIABLE << std::endl;
        const char* dcmDictPath = getenv(DCM_DICT_ENVIRONMENT_VARIABLE);
        if (dcmDictPath == NULL) {
            std::cout << "환경 변수 " << DCM_DICT_ENVIRONMENT_VARIABLE << "가 설정되지 않았습니다." << std::endl;
        }
        else {
            std::cout << "사전 경로: " << dcmDictPath << std::endl;
        }
    }

    DcmFileFormat fileformat;
    OFCondition status = fileformat.loadFile(filename, EXS_Unknown, EGL_noChange, DCM_MaxReadLength, ERM_autoDetect);

    if (status.good()) {
        std::cout << "파일을 성공적으로 로드했습니다." << std::endl;
        DcmDataset* dataset = fileformat.getDataset();
        DcmMetaInfo* metainfo = fileformat.getMetaInfo();

        // 전송 구문 확인 (메타 정보에서)
        if (metainfo) {
            const char* transferSyntaxUID = NULL;
            if (metainfo->findAndGetString(DCM_TransferSyntaxUID, transferSyntaxUID).good() && transferSyntaxUID) {
                std::cout << "전송 구문 UID (메타 정보): " << transferSyntaxUID << std::endl;
            }
        }

        // 전송 구문 확인 (데이터셋에서)
        E_TransferSyntax xfer = dataset->getOriginalXfer();
        std::cout << "전송 구문 UID (데이터셋): " << DcmXfer(xfer).getXferID() << std::endl;
        std::cout << "전송 구문 이름: " << DcmXfer(xfer).getXferName() << std::endl;

        // DICOM 파일의 이미지를 DicomImage 클래스로 로드
        DicomImage* image = new DicomImage(dataset, xfer);
        if (image != NULL && image->getStatus() == EIS_Normal) {
            std::cout << "이미지 너비: " << image->getWidth() << std::endl;
            std::cout << "이미지 높이: " << image->getHeight() << std::endl;
            delete image;
        }
        else {
            std::cerr << "Error: cannot load DICOM image (" << DicomImage::getString(image->getStatus()) << ")" << std::endl;
        }
    }
    else {
        std::cerr << "Error: cannot read DICOM file (" << status.text() << ")" << std::endl;
        std::cerr << "Error code: " << status.code() << std::endl;
        std::cerr << "Error module: " << status.module() << std::endl;
    }

    // 초기화 해제
    DJDecoderRegistration::cleanup();
    DcmRLEDecoderRegistration::cleanup();

    return 0;
}
