#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmimgle/dcmimage.h"
#include "dcmtk/dcmjpeg/djdecode.h"
#include "dcmtk/dcmdata/dcrledrg.h"

// 비표준 VR 무시 설정
OFCondition enableUnknownVRReading() {
    dcmEnableUnknownVRGeneration.set(OFTrue);
    return EC_Normal;
}

// 비표준 태그 무시 설정
OFCondition enableUnknownTagReading() {
    dcmIgnoreParsingErrors.set(OFTrue);
    return EC_Normal;
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

    // 비표준 VR 및 태그 무시 설정 호출
    enableUnknownVRReading();
    enableUnknownTagReading();

    const char* filename = "D:\\dicom\\myAngio.dcm"; // 읽을 DICOM 파일 경로
    DcmFileFormat fileformat;

    OFCondition status = fileformat.loadFile(filename, EXS_Unknown, EGL_withoutGL, DCM_MaxReadLength, ERM_autoDetect);

    if (status.good()) {
        DcmDataset* dataset = fileformat.getDataset();

        // DICOM 파일의 이미지를 DicomImage 클래스로 로드
        DicomImage* image = new DicomImage(dataset, EXS_Unknown);
        if (image != nullptr && image->getStatus() == EIS_Normal) {
            // 이미지 처리 로직 추가 (예: 저장, 표시 등)
            // 예시로 이미지의 너비와 높이를 출력
            std::cout << "Image width: " << image->getWidth() << std::endl;
            std::cout << "Image height: " << image->getHeight() << std::endl;

            // 이미지 해제
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
