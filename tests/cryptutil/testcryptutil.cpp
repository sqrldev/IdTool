#include "testcryptutil.h"

void TestCryptUtil::createSiteKeys()
{
    QString str = "Hello";
    QVERIFY(str.toUpper() == "HELLO");
}

QTEST_MAIN(TestCryptUtil)
