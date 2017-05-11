#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define CLAMP(a, b, c) (MIN(MAX(a, b), c))

struct Vec2
{
    double x;
    double y;
    inline Vec2 operator+(const Vec2 & other) const { return {x+other.x, y+other.y}; }
    inline Vec2 operator-(const Vec2 & other) const { return {x-other.x, y-other.y}; }
    inline Vec2 operator*(double scalar) const { return {x*scalar, y*scalar}; }
    inline Vec2 operator/(double scalar) const { return {x/scalar, y/scalar}; }
    inline void operator+=(const Vec2 & other) { x += other.x, y += other.y; }
    inline void operator-=(const Vec2 & other) { x -= other.x, y -= other.y; }
    inline void operator*=(double scalar) { x *= scalar, y *= scalar; }
    inline void operator/=(double scalar) { x /= scalar, y /= scalar; }
    inline double dot(const Vec2 & other) const { return x*other.x + y*other.y; }
    inline double cross(const Vec2 & other) const { return x*other.y - y*other.x; }
    inline double norm2() const { return x*x+y*y; }
    inline double norm() const { return sqrt(norm2()); }
    inline double angle() const { return atan2(x,y); }
    inline double angle(const Vec2 & other) const { double res = angle() - other.angle(); if (res < -M_PI) res += 2.*M_PI; if (res > M_PI) res -= 2.*M_PI; return res; }
    inline Vec2 normalized() const { return operator/(norm()); }
    inline void normalize() { *this = normalized(); }
    inline Vec2 rotated(double angle) const { return { x*cos(angle)+y*sin(angle), -x*sin(angle)+y*cos(angle) }; }
    inline void rotate(double angle) { *this = rotated(angle); }
    static const Vec2 unit;
};
ostream & operator<<(ostream & o, const Vec2 & v)
{
    return o << '[' << v.x << ", " << v.y << ']';
}
inline Vec2 operator*(double scalar, const Vec2 & vec) { return {vec.x*scalar, vec.y*scalar}; }
inline Vec2 operator/(double scalar, const Vec2 & vec) { return {vec.x/scalar, vec.y/scalar}; }
const Vec2 Vec2::unit = { 1, 0 };

inline double hypot2(double dx, double dy)
{
    return dx*dx+dy*dy;
}

struct Segment
{
    Vec2 p1;
    Vec2 p2;
    inline Vec2 vec() { return p2-p1; }
    inline double dot(Segment & other) { return vec().dot(other.vec()); }
    inline double cross(Segment & other) { return vec().cross(other.vec()); }
    inline double norm2() { return vec().norm2(); }
    inline double norm() { return vec().norm(); }
};
ostream & operator<<(ostream & o, const Segment & s)
{
    return o << '[' << s.p1 << ", " << s.p2 << ']';
}

bool lineLineIntersection(const Segment & s1, const Segment & s2, Vec2 * res = NULL)
{
    Vec2 u = s1.p2 - s1.p1;
    Vec2 v = s2.p2 - s2.p1;
    double D = u.cross(v);
    if (D == 0) return false;
    if (res)
    {
        Vec2 w = s1.p1 - s2.p1;
        *res = s1.p1 + v.cross(w) / D * u;
    }
    return true;
}

bool lineSegmentIntersection(const Segment & s1, const Segment & s2, Vec2 * res = NULL)
{
    Vec2 u = s1.p2 - s1.p1;
    Vec2 v = s2.p2 - s2.p1;
    Vec2 w = s1.p1 - s2.p1;
    double D = u.cross(v);
    if (D == 0) return false;
    double ucrossw = u.cross(w) / D;
    if (ucrossw < 0 || ucrossw > 1)
        return false;
    if (res)
        *res = s1.p1 + v.cross(w) / D * u;
    return true;
}

bool segmentSegmentIntersection(const Segment & s1, const Segment & s2, Vec2 * res = NULL)
{
    Vec2 u = s1.p2 - s1.p1;
    Vec2 v = s2.p2 - s2.p1;
    Vec2 w = s1.p1 - s2.p1;
    double D = u.cross(v);
    if (D == 0) return false;
    double vcrossw = v.cross(w) / D;
    if (vcrossw < 0 || vcrossw > 1)
        return false;
    double ucrossw = u.cross(w) / D;
    if (ucrossw < 0 || ucrossw > 1)
        return false;
    if (res)
        *res = s1.p1 + vcrossw * u;
    return true;
}

inline double project(Segment & s, Vec2 & p)
{
    double dx12 = s.p2.x - s.p1.x;
    double dy12 = s.p2.y - s.p1.y;
    return ((p.x - s.p1.x) * dx12 + (p.y - s.p1.y) * dy12) / hypot2(dx12, dy12);
}

double lineVec2Distance2(Segment & s, Vec2 & p)
{
    double proj = project(s, p);
    double cx = s.p1.x + proj * (s.p2.x-s.p1.x);
    double cy = s.p1.y + proj * (s.p2.y-s.p1.y);
    return (p.x-cx) * (p.x-cx) + (p.y-cy) * (p.y-cy);
}

double segmentVec2Distance2(Segment & s, Vec2 & p)
{
    double proj = project(s, p);
    if (proj < 0.) proj = 0.;
    else if (proj > 1.) proj = 1.;
    double cx = s.p1.x + proj * (s.p2.x-s.p1.x);
    double cy = s.p1.y + proj * (s.p2.y-s.p1.y);
    return (p.x-cx) * (p.x-cx) + (p.y-cy) * (p.y-cy);
}
