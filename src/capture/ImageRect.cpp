/*********************************************************
 * ImageRect.cpp                                         *
 *                                                       *
 * Include CapPoint, Rect, ImageRect class               *
 *                                                       *
 *********************************************************/
#include <algorithm>
#include "ImageRect.h"
#include "WindowProcessor.h"

bool CapPoint::isEqual(const CapPoint& a)
{
    return (getX() == a.getX()) && (getY()== a.getY());
}

bool CapPoint::isLess (const CapPoint& a) const
{
    return (getX() <= a.getX()) && (getY() <= a.getY());
}

bool CapPoint::isGreat (const CapPoint& a) const
{
    return (getX() >= a.getX()) && (getY() >= a.getY());
}

std::ostream& operator<< (std::ostream& os, const CapPoint& p)
{
    os << "{ "
       << p.getX() << ", " << p.getY()
       << " }";
    return os;
}

CapImageRect::CapImageRect(int l, int t, int r, int b)
{
    _lt.setX(l);
    _lt.setY(t);
    _rb.setX(r);
    _rb.setY(b);
    _size.setX(r-l);
    _size.setY(b-t);
    _isValid = true;
}

void CapImageRect::set(int l, int t, int r, int b)
{
    _lt.setX(l);
    _lt.setY(t);
    _rb.setX(r);
    _rb.setY(b);
    _size.setX(r-l);
    _size.setY(b-t);
    _isValid = true;
}

CapMonitor CapMonitor::getById(int id)
{
    MonitorVectorProvider mvp;
    auto result = std::find_if(mvp.get().begin(), mvp.get().end(), [=](const CapMonitor & mon) -> bool {
        return mon.getId() == id;
    });

    return (result == mvp.get().end()) ? CapMonitor() : *result;
}

CapWindow CapWindow::getWinById(size_t h)
{
    WindowVectorProvider wvp(-1);
    auto result = std::find_if(wvp.get().begin(), wvp.get().end(),
                               [=](const CapWindow & win) -> bool {
                    return win.getHandler() == h;
    });

    return (result == wvp.get().end()) ? CapWindow() : *result;
}