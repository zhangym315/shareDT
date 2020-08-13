#ifndef _IMAGERECT_H_
#define _IMAGERECT_H_

#include <string>
#include <iostream>
#include <vector>
#include "TypeDef.h"
enum SPWinType { SP_WIN_PROCESS, SP_WIN_HANDLER, SP_WIN_NULL};

class CapPoint {
  public:
    CapPoint(const CapPoint & a) { setX(a.getX()); setY(a.getY()); }
    CapPoint(int x, int y) : _x(x), _y(y) { }
    CapPoint() : CapPoint (0, 0) { }

    int getX() const { return _x; }
    int getY() const { return _y; }

    void setX(const int x) { _x = x; }
    void setY(const int y) { _y = y; }

    CapPoint & operator = (CapPoint & src) {
        this->setX(src.getX ());
        this->setY(src.getY ());
        return *this;
    }

    bool isEqual(const CapPoint & a);

    /*
     * A point is less than a point when both _x and _y is less
     * A point is greater than a pointer, when both _x and _y are greater
     */
    bool isLess (const CapPoint & a) const;
    bool isGreat(const CapPoint & a) const;

    /* overloading = */
    CapPoint & operator = (const CapPoint & other) {
        _x = other.getX ();
        _y = other.getY();
        return *this;
    }
  private:
    int _x;
    int _y;
};

std::ostream& operator<< (std::ostream& os, const CapPoint& p);

struct CapImageBGRA {
    unsigned char B, G, R, A;
};

class CapImageRect {
  public:
    CapImageRect() { }
    //: CapImageRect(0, 0, 0, 0) {}
    CapImageRect(int l, int t, int r, int b) ;

    void set(int l, int t, int r, int b);

    const CapPoint & getLT() const { return _lt; }
    const CapPoint & getRB() const { return _rb; }
    const CapPoint & getSZ() const { return _size; }

    bool Contains(const CapImageRect &a) const { return _lt.isLess(a.getLT()) && _rb.isGreat (a.getRB()); }

    bool isEqual(const CapImageRect &a)
    {
        return _lt.isEqual(a.getLT()) && _rb.isEqual(a.getRB());
    }

    int getWidth() const { return _size.getX(); }
    int getHeight()const { return _size.getY(); }
    int getTLX() const { return _lt.getX(); }
    int getTLY() const { return _lt.getY(); }

    /* overloading = */
    CapImageRect & operator = (const CapImageRect & other) {
        _lt = other.getLT ();
        _rb = other.getRB();
        _size = other.getSZ();
        return *this;
    }

    void setInvalid() { _isValid = false; }
    bool isValid() { return _isValid; }

    String toString(char * delimiter) {
        return std::to_string(_lt.getX()) + String(delimiter) +
                std::to_string(_lt.getY()) + String(delimiter) +
                std::to_string(_rb.getX()) + String(delimiter) +
                std::to_string(_rb.getY());
    }

  private:
    CapPoint _lt;     /* left top point     */
    CapPoint _rb;     /* right bottom point */
    CapPoint _size ;  /* width and height,
                       * notice not a point */
    bool     _isValid;
};


struct Image {
    CapImageRect bounds;
    int       bytesToNextRow = 0;
    bool      isContiguous   = false;
    // alpha is always unused and might contain garbage
    const CapImageBGRA *Data = nullptr;
};

class CapWindow {
  public:
    CapWindow(size_t h, CapPoint & o, CapPoint &s, String n, Pid p) : _handler(h),
            _offset(o),  _size(s), _name(n), _pid(p), _isValid (true)  {  }
    CapWindow() { _isValid = false; }
    ~CapWindow () { }
    size_t getHandler() const { return _handler; }
    Pid    getPid    () const { return _pid; }
    bool   isValid   () const { return _isValid; }
    void   setInvalid() { _isValid = true; }
    const CapPoint & getOffset() const { return _offset; }
    const CapPoint & getSize () const { return _size; }
    const String   & getName () const { return _name; }
    int   getWidth()  const { return _size.getX (); }
    int   getHeight() const { return _size.getY (); }
    void  clear() { _all.clear(); }
    void  push(size_t h) { _all.push_back(h); }
    std::vector<size_t> getAll() const { return _all; }

    void operator = (const CapWindow &other) {
        _handler = other.getHandler ();
        _offset    = other.getOffset ();
        _size = other.getSize();
        _name  = other.getName ();
        _pid = other.getPid ();
        _isValid = other.isValid ();
        _all = other.getAll ();
    }

    void setOffset(int x, int y) { _offset.setX(x); _offset.setY(y);}
    void setSize (int x, int y) { _size.setX(x); _size.setY(y);}

    void setWinType(SPWinType wintype) { _winType = wintype; }
    SPWinType getWinType() { return _winType; }

  private:
    size_t    _handler;
    SPWinType _winType;
    std::vector<size_t>  _all;   /* all handler with the same process id */
    CapPoint  _offset;
    CapPoint  _size;
    String    _name;
    Pid       _pid;
    bool      _isValid;
};

class CapMonitor {
  public:
    CapMonitor(int idx, int id, CapPoint sz, CapPoint of, float sc) :
        _index(idx), _id(id), _size(sz), _offset(of), _scale(sc), _isValid(true){
        _orgSize  = _size;
        _orgOffset = _offset;
        _name = String("Display-") + std::to_string(idx);
    }
    CapMonitor(int idx, int id, CapPoint sz, CapPoint of, CapPoint os, float sc) :
        _index(idx), _id(id), _size(sz), _offset(of), _orgSize(os),
        _scale(sc), _isValid(true){
         _orgOffset = _offset;
        _name = String("Display-") + std::to_string(idx);
    }
    CapMonitor(int idx, int id, int w, int h, int ox, int oy, float sc) :
        CapMonitor (idx, id, CapPoint(w, h), CapPoint(ox, oy), sc)  { }

    CapMonitor(int idx, int id, int w, int h, int ox, int oy,
                    int ow, int oh, float sc) :
            CapMonitor (idx, id, CapPoint(w, h), CapPoint(ox, oy), CapPoint(ow, oh), sc)  { }
    CapMonitor(int idx, int id, int w, int h, int ox, int oy, float sc, int adapter) :
            CapMonitor (idx, id, CapPoint(w, h), CapPoint(ox, oy), sc)  {
        _adapter = adapter;
        _isValid = true;
    }
    CapMonitor(String & name, int idx, int id, int w, int h, int ox, int oy, float sc, int adapter) :
            CapMonitor (idx, id, CapPoint(w, h), CapPoint(ox, oy), sc)  {
        _adapter = adapter;
        _isValid = true;
        _name    = name;
    }

    CapMonitor() : CapMonitor (0, 0, CapPoint(0, 0), CapPoint(0, 0), 0) { _isValid = false; }
    ~CapMonitor() { }

    int   getId()    const { return _id ;   }
    int   getIndex() const { return _index; }
    float  getScale() const { return _scale; }
    int   getAdapter() const { return _adapter; }
    const String  & getName()  const { return _name ; }
    const CapPoint& getOffset() const { return _offset; }
    const CapPoint& getSize () const  { return _size ; }
    const CapPoint& getOrgOffset() const { return _orgOffset; }
    const CapPoint& getOrgSize () const { return _orgSize; }
    int   getOrgHeight() { return _orgSize.getY(); }
    int   getOrgWidth()  { return _orgSize.getX(); }
    int   getWidth() { return _size.getX (); }
    int   getHeight() { return _size.getY (); }
    bool  isValid() const { return _isValid; }
    void  setInValid() { _isValid = false; }

    void operator = (const CapMonitor &other) {
        _index = other.getIndex ();
        _id    = other.getId ();
        _adapter = other.getAdapter();
        _name  = other.getName ();
        _scale = other.getScale ();

        _offset = other.getOffset ();
        _size  = other.getSize ();
        _orgOffset = other.getOrgOffset();
        _orgSize  = other.getOrgSize();
        _isValid  = other.isValid ();
    }

  private:
    int _index;
    int _id;
    int _adapter;

    CapPoint _offset;
    CapPoint _size ;    /* Width and height */

    CapPoint _orgOffset;
    CapPoint _orgSize;  /* Width and height */

    String  _name;
    float _scale = 1.000f;

    bool _isValid;
};

typedef std::vector<CapWindow> WindowVector;
typedef std::vector<CapMonitor> MonitorVector;


#endif //_IMAGERECT_H_
