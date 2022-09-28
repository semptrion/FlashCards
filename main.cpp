#include "flashcard.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FlashCard w;
    w.show();
    return a.exec();
}
