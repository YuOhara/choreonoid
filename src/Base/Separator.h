/**
   @author Shin'ichiro NAKAOKA
*/

#ifndef CNOID_GUIBASE_SEPARATOR_H_INCLUDED
#define CNOID_GUIBASE_SEPARATOR_H_INCLUDED

#include <QFrame>
#include <QBoxLayout>

namespace cnoid {

class VSeparator : public QFrame
{
public:
    inline VSeparator(QWidget* parent = 0) : QFrame(parent) {
        setFrameStyle(QFrame::VLine | QFrame::Sunken);
    }
};

class HSeparator : public QFrame
{
public:
    inline HSeparator(QWidget* parent = 0) : QFrame(parent) {
        setFrameStyle(QFrame::HLine | QFrame::Sunken);
    }
};

class HSeparatorBox : public QHBoxLayout
{
public:
    HSeparatorBox(QWidget* titleWidget) {
        addWidget(new HSeparator(), 1);
        addWidget(titleWidget);
        addWidget(new HSeparator(), 1);
    }
};
}

#endif
