#ifndef S4_HPP
#define S4_HPP

#include <stdint.h>
#include <vector>

namespace SqrlStorage
{

    /*!
        \class S4

        \brief The S4 class provides access to the
        "SQRL Secure Storage System" (S4).
    */
    class S4
    {
    public:
        static const int HEADER_SIZE;
        static const char* HEADER;
        static const char* HEADER_BASE64;
        class Block; //Forward decl
        std::vector<Block*> blocks;

    private:
        bool m_IsBase64 = false;
        void parse(std::vector<char> *data);

    public:

        void readIdentityFile(char* fileName);
        void readIdentityText(char* identityData, int length);
        bool checkHeader(std::vector<char> *data);
        ~S4();


        class Block
        {
        public:
            uint16_t length = 0;
            uint16_t type = 0;

            Block(std::vector<char>* data, int blockStartIndex);
        };

        class BlockType1 : public Block
        {
        public:
            uint16_t plainTextLength = 0;
            unsigned char aesGcmIv[12] = { 0 };
            unsigned char scryptRandomSalt[16] = { 0 };
            unsigned char scryptLogNFactor = 0;
            uint32_t scryptIterationCount = 0;
            uint16_t optionFlags = 0;
            unsigned char hintLength = 0;
            unsigned char pwVerifySecs = 0;
            uint16_t idleTimeout = 0;
            unsigned char identityMasterKey[32] = { 0 };
            unsigned char identityLockKey[32] = { 0 };
            unsigned char verificationTag[16] = { 0 };

            BlockType1(std::vector<char>* data, int blockStartIndex);
        };

        class BlockType2 : public Block
        {
        public:
            unsigned char scryptRandomSalt[16] = { 0 };
            unsigned char scryptLogNFactor = 0;
            uint32_t scryptIterationCount = 0;
            unsigned char identityUnlockKey[32] = { 0 };
            unsigned char verificationTag[16] = { 0 };

            BlockType2(std::vector<char>* data, int blockStartIndex);
        };

        class BlockType3 : public Block
        {
        public:
            uint16_t nrOfPreviousIuks = 0;
            std::vector<char*> previousIuks;
            unsigned char verificationTag[16] = { 0 };

            BlockType3(std::vector<char>* data, int blockStartIndex);
            ~BlockType3();
        };

        enum EBlockType
        {
            TYPE_1_PW_ENCRYPTED_DATA = 1,
            TYPE_2_RC_ENCRYPTED_DATA = 2,
            TYPE_3_PREVIOUS_IUK = 3
        };
    };
}

#endif // S4_HPP
