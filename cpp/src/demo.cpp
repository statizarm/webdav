#include "cpp/include/demo.h"
#define TEST_FILE_NAME "/test"
#define TEST_FILE_VALUE "hello, world!!!"
#define TEST_DIR_NAME "/test/"


Demo::Demo (QWidget *parent) :
    QWidget (parent)
{
    wd = new WebDavManager (this);

    wd->setConnectionSettings (WebDavManager::ConnectionType::HTTPS,
                               "webdav.yandex.ru",
                               "/",
                               "login",
                               "pswrd",
                               443);

    vertLayout = new QVBoxLayout (this);

    putTestButton = new QPushButton ("Test put cmd");
    getTestButton = new QPushButton ("Test get cmd");
    removeTestButton = new QPushButton ("Test remove cmd");
    mkdirTestButton = new QPushButton ("Test mkdir cmd");
    moveTestButton = new QPushButton ("Test move cmd");
    copyTestButton = new QPushButton ("Test copy cmd");
    testLabel = new QLabel ("hmmm");

    vertLayout->addWidget (putTestButton);
    vertLayout->addWidget (getTestButton);
    vertLayout->addWidget (removeTestButton);
    vertLayout->addWidget (mkdirTestButton);
    vertLayout->addWidget (moveTestButton);
    vertLayout->addWidget (copyTestButton);
    vertLayout->addWidget (testLabel);

    connect (putTestButton, SIGNAL (clicked ()), this, SLOT (testPutCmd ()));
    connect (getTestButton, SIGNAL (clicked ()), this, SLOT (testGetCmd ()));
    connect (removeTestButton, SIGNAL (clicked ()), this, SLOT (testRemoveCmd ()));
    connect (mkdirTestButton, SIGNAL (clicked ()), this, SLOT (testMkdirCmd ()));
    connect (moveTestButton, SIGNAL (clicked ()), this, SLOT (testMoveCmd ()));
    connect (copyTestButton, SIGNAL (clicked ()), this, SLOT (testCopyCmd ()));
}

void
Demo::testPutCmd ()
{
    wd->put(TEST_FILE_NAME, TEST_FILE_VALUE);
}

void
Demo::testGetCmd ()
{
    QIODevice *buf = new QBuffer;
    wd->get (TEST_FILE_NAME, buf);
    testLabel->setText ("hmm");
}

void
Demo::testRemoveCmd ()
{
    wd->remove (TEST_FILE_NAME);
    wd->remove (TEST_DIR_NAME);
}

void
Demo::testMkdirCmd ()
{
    wd->mkdir (TEST_DIR_NAME);
}

void
Demo::testMoveCmd ()
{
    wd->move (TEST_FILE_NAME, TEST_DIR_NAME TEST_FILE_NAME);
}

void
Demo::testCopyCmd ()
{
    wd->copy (TEST_FILE_NAME, TEST_DIR_NAME TEST_FILE_NAME);
}
