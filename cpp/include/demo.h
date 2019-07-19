#ifndef DEMO_H_
#define DEMO_H_

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include "cpp/include/webdav.h"
#include <QLabel>

class Demo : public QWidget
{
    Q_OBJECT
public:
    Demo (QWidget *parent = nullptr);
private:
    WebDavManager *wd;
    QVBoxLayout *vertLayout;
    QPushButton *putTestButton;
    QPushButton *getTestButton;
    QPushButton *removeTestButton;
    QPushButton *mkdirTestButton;
    QPushButton *moveTestButton;
    QPushButton *copyTestButton;
    QLabel *testLabel;
protected slots:
    void testPutCmd ();
    void testGetCmd ();
    void testRemoveCmd ();
    void testMkdirCmd ();
    void testMoveCmd ();
    void testCopyCmd ();
};

#endif // DEMO_H_
