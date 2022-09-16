#ifndef IEMITRESIZE_H
#define IEMITRESIZE_H

#include<QtPlugin>
#include<QObject>

class IEmitResize
{
public:
    virtual ~ IEmitResize(){};

signals:
    virtual void resized(int, int) = 0;
};

Q_DECLARE_INTERFACE(IEmitResize, "IEmitResize");

#endif // IEMITRESIZE_H
