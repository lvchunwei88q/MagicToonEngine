// Dear ImGui implementation - see LICENSE.txt.
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui.h"
#ifndef IMGUI_DISABLE
#include "imgui_internal.h"
#include "imgui_platform_private.h"
#include <stdio.h>

//-----------------------------------------------------------------------------
// [SECTION] MISC HELPERS/UTILITIES (Geometry functions)
//-----------------------------------------------------------------------------

ImVec2 ImBezierCubicClosestPoint(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, const ImVec2& p, int num_segments)
{
    IM_ASSERT(num_segments > 0); // Use ImBezierCubicClosestPointCasteljau()
    ImVec2 p_last = p1;
    ImVec2 p_closest;
    float p_closest_dist2 = FLT_MAX;
    float t_step = 1.0f / (float)num_segments;
    for (int i_step = 1; i_step <= num_segments; i_step++)
    {
        ImVec2 p_current = ImBezierCubicCalc(p1, p2, p3, p4, t_step * i_step);
        ImVec2 p_line = ImLineClosestPoint(p_last, p_current, p);
        float dist2 = ImLengthSqr(p - p_line);
        if (dist2 < p_closest_dist2)
        {
            p_closest = p_line;
            p_closest_dist2 = dist2;
        }
        p_last = p_current;
    }
    return p_closest;
}

// Closely mimics PathBezierToCasteljau() in imgui_draw.cpp
static void ImBezierCubicClosestPointCasteljauStep(const ImVec2& p, ImVec2& p_closest, ImVec2& p_last, float& p_closest_dist2, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float tess_tol, int level)
{
    float dx = x4 - x1;
    float dy = y4 - y1;
    float d2 = ((x2 - x4) * dy - (y2 - y4) * dx);
    float d3 = ((x3 - x4) * dy - (y3 - y4) * dx);
    d2 = (d2 >= 0) ? d2 : -d2;
    d3 = (d3 >= 0) ? d3 : -d3;
    if ((d2 + d3) * (d2 + d3) < tess_tol * (dx * dx + dy * dy))
    {
        ImVec2 p_current(x4, y4);
        ImVec2 p_line = ImLineClosestPoint(p_last, p_current, p);
        float dist2 = ImLengthSqr(p - p_line);
        if (dist2 < p_closest_dist2)
        {
            p_closest = p_line;
            p_closest_dist2 = dist2;
        }
        p_last = p_current;
    }
    else if (level < 10)
    {
        float x12 = (x1 + x2)*0.5f,       y12 = (y1 + y2)*0.5f;
        float x23 = (x2 + x3)*0.5f,       y23 = (y2 + y3)*0.5f;
        float x34 = (x3 + x4)*0.5f,       y34 = (y3 + y4)*0.5f;
        float x123 = (x12 + x23)*0.5f,    y123 = (y12 + y23)*0.5f;
        float x234 = (x23 + x34)*0.5f,    y234 = (y23 + y34)*0.5f;
        float x1234 = (x123 + x234)*0.5f, y1234 = (y123 + y234)*0.5f;
        ImBezierCubicClosestPointCasteljauStep(p, p_closest, p_last, p_closest_dist2, x1, y1, x12, y12, x123, y123, x1234, y1234, tess_tol, level + 1);
        ImBezierCubicClosestPointCasteljauStep(p, p_closest, p_last, p_closest_dist2, x1234, y1234, x234, y234, x34, y34, x4, y4, tess_tol, level + 1);
    }
}

// tess_tol is generally the same value you would find in ImGui::GetStyle().CurveTessellationTol
// Because those ImXXX functions are lower-level than ImGui:: we cannot access this value automatically.
ImVec2 ImBezierCubicClosestPointCasteljau(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, const ImVec2& p, float tess_tol)
{
    IM_ASSERT(tess_tol > 0.0f);
    ImVec2 p_last = p1;
    ImVec2 p_closest;
    float p_closest_dist2 = FLT_MAX;
    ImBezierCubicClosestPointCasteljauStep(p, p_closest, p_last, p_closest_dist2, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, tess_tol, 0);
    return p_closest;
}

ImVec2 ImLineClosestPoint(const ImVec2& a, const ImVec2& b, const ImVec2& p)
{
    ImVec2 ap = p - a;
    ImVec2 ab_dir = b - a;
    float dot = ap.x * ab_dir.x + ap.y * ab_dir.y;
    if (dot < 0.0f)
        return a;
    float ab_len_sqr = ab_dir.x * ab_dir.x + ab_dir.y * ab_dir.y;
    if (dot > ab_len_sqr)
        return b;
    return a + ab_dir * dot / ab_len_sqr;
}

bool ImTriangleContainsPoint(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& p)
{
    bool b1 = ((p.x - b.x) * (a.y - b.y) - (p.y - b.y) * (a.x - b.x)) < 0.0f;
    bool b2 = ((p.x - c.x) * (b.y - c.y) - (p.y - c.y) * (b.x - c.x)) < 0.0f;
    bool b3 = ((p.x - a.x) * (c.y - a.y) - (p.y - a.y) * (c.x - a.x)) < 0.0f;
    return (b1 == b2) && (b2 == b3);
}

void ImTriangleBarycentricCoords(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& p, float& out_u, float& out_v, float& out_w)
{
    ImVec2 v0 = b - a;
    ImVec2 v1 = c - a;
    ImVec2 v2 = p - a;
    const float denom = v0.x * v1.y - v1.x * v0.y;
    out_v = (v2.x * v1.y - v1.x * v2.y) / denom;
    out_w = (v0.x * v2.y - v2.x * v0.y) / denom;
    out_u = 1.0f - out_v - out_w;
}

ImVec2 ImTriangleClosestPoint(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& p)
{
    ImVec2 proj_ab = ImLineClosestPoint(a, b, p);
    ImVec2 proj_bc = ImLineClosestPoint(b, c, p);
    ImVec2 proj_ca = ImLineClosestPoint(c, a, p);
    float dist2_ab = ImLengthSqr(p - proj_ab);
    float dist2_bc = ImLengthSqr(p - proj_bc);
    float dist2_ca = ImLengthSqr(p - proj_ca);
    float m = ImMin(dist2_ab, ImMin(dist2_bc, dist2_ca));
    if (m == dist2_ab)
        return proj_ab;
    if (m == dist2_bc)
        return proj_bc;
    return proj_ca;
}

//-----------------------------------------------------------------------------
// [SECTION] MISC HELPERS/UTILITIES (String, Format, Hash functions)
//-----------------------------------------------------------------------------

// Consider using _stricmp/_strnicmp under Windows or strcasecmp/strncasecmp. We don't actually use either ImStricmp/ImStrnicmp in the codebase any more.
int ImStricmp(const char* str1, const char* str2)
{
    int d;
    while ((d = ImToUpper(*str2) - ImToUpper(*str1)) == 0 && *str1) { str1++; str2++; }
    return d;
}

int ImStrnicmp(const char* str1, const char* str2, size_t count)
{
    int d = 0;
    while (count > 0 && (d = ImToUpper(*str2) - ImToUpper(*str1)) == 0 && *str1) { str1++; str2++; count--; }
    return d;
}

void ImStrncpy(char* dst, const char* src, size_t count)
{
    if (count < 1)
        return;
    if (count > 1)
        strncpy(dst, src, count - 1); // FIXME-OPT: strncpy not only doesn't guarantee 0-termination, it also always writes the whole array
    dst[count - 1] = 0;
}

char* ImStrdup(const char* str)
{
    size_t len = ImStrlen(str);
    void* buf = IM_ALLOC(len + 1);
    return (char*)memcpy(buf, (const void*)str, len + 1);
}

void* ImMemdup(const void* src, size_t size)
{
    void* dst = IM_ALLOC(size);
    return memcpy(dst, src, size);
}

char* ImStrdupcpy(char* dst, size_t* p_dst_size, const char* src)
{
    size_t dst_buf_size = p_dst_size ? *p_dst_size : ImStrlen(dst) + 1;
    size_t src_size = ImStrlen(src) + 1;
    if (dst_buf_size < src_size)
    {
        IM_FREE(dst);
        dst = (char*)IM_ALLOC(src_size);
        if (p_dst_size)
            *p_dst_size = src_size;
    }
    return (char*)memcpy(dst, (const void*)src, src_size);
}

const char* ImStrchrRange(const char* str, const char* str_end, char c)
{
    const char* p = (const char*)ImMemchr(str, (int)c, str_end - str);
    return p;
}

int ImStrlenW(const ImWchar* str)
{
    //return (int)wcslen((const wchar_t*)str);  // FIXME-OPT: Could use this when wchar_t are 16-bit
    int n = 0;
    while (*str++) n++;
    return n;
}

// Find end-of-line. Return pointer will point to either first \n, either str_end.
const char* ImStreolRange(const char* str, const char* str_end)
{
    const char* p = (const char*)ImMemchr(str, '\n', str_end - str);
    return p ? p : str_end;
}

const char* ImStrbol(const char* buf_mid_line, const char* buf_begin) // find beginning-of-line
{
    IM_ASSERT_PARANOID(buf_mid_line >= buf_begin && buf_mid_line <= buf_begin + ImStrlen(buf_begin));
    while (buf_mid_line > buf_begin && buf_mid_line[-1] != '\n')
        buf_mid_line--;
    return buf_mid_line;
}

const char* ImStristr(const char* haystack, const char* haystack_end, const char* needle, const char* needle_end)
{
    if (!needle_end)
        needle_end = needle + ImStrlen(needle);

    const char un0 = (char)ImToUpper(*needle);
    while ((!haystack_end && *haystack) || (haystack_end && haystack < haystack_end))
    {
        if (ImToUpper(*haystack) == un0)
        {
            const char* b = needle + 1;
            for (const char* a = haystack + 1; b < needle_end; a++, b++)
                if (ImToUpper(*a) != ImToUpper(*b))
                    break;
            if (b == needle_end)
                return haystack;
        }
        haystack++;
    }
    return NULL;
}

// Trim str by offsetting contents when there's leading data + writing a \0 at the trailing position. We use this in situation where the cost is negligible.
void ImStrTrimBlanks(char* buf)
{
    char* p = buf;
    while (p[0] == ' ' || p[0] == '\t')     // Leading blanks
        p++;
    char* p_start = p;
    while (*p != 0)                         // Find end of string
        p++;
    while (p > p_start && (p[-1] == ' ' || p[-1] == '\t'))  // Trailing blanks
        p--;
    if (p_start != buf)                     // Copy memory if we had leading blanks
        memmove(buf, p_start, p - p_start);
    buf[p - p_start] = 0;                   // Zero terminate
}

const char* ImStrSkipBlank(const char* str)
{
    while (str[0] == ' ' || str[0] == '\t')
        str++;
    return str;
}

// A) MSVC version appears to return -1 on overflow, whereas glibc appears to return total count (which may be >= buf_size).
// Ideally we would test for only one of those limits at runtime depending on the behavior the vsnprintf(), but trying to deduct it at compile time sounds like a pandora can of worm.
// B) When buf==NULL vsnprintf() will return the output size.
#ifndef IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS

// We support stb_sprintf which is much faster (see: https://github.com/nothings/stb/blob/master/stb_sprintf.h)
// You may set IMGUI_USE_STB_SPRINTF to use our default wrapper, or set IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS
// and setup the wrapper yourself. (FIXME-OPT: Some of our high-level operations such as ImGuiTextBuffer::appendfv() are
// designed using two-passes worst case, which probably could be improved using the stbsp_vsprintfcb() function.)
#ifdef IMGUI_USE_STB_SPRINTF
#ifndef IMGUI_DISABLE_STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_IMPLEMENTATION
#endif
#ifdef IMGUI_STB_SPRINTF_FILENAME
#include IMGUI_STB_SPRINTF_FILENAME
#else
#include "stb_sprintf.h"
#endif
#endif // #ifdef IMGUI_USE_STB_SPRINTF

#if defined(_MSC_VER) && !defined(vsnprintf)
#define vsnprintf _vsnprintf
#endif

int ImFormatString(char* buf, size_t buf_size, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
#ifdef IMGUI_USE_STB_SPRINTF
    int w = stbsp_vsnprintf(buf, (int)buf_size, fmt, args);
#else
    int w = vsnprintf(buf, buf_size, fmt, args);
#endif
    va_end(args);
    if (buf == NULL)
        return w;
    if (w == -1 || w >= (int)buf_size)
        w = (int)buf_size - 1;
    buf[w] = 0;
    return w;
}

int ImFormatStringV(char* buf, size_t buf_size, const char* fmt, va_list args)
{
#ifdef IMGUI_USE_STB_SPRINTF
    int w = stbsp_vsnprintf(buf, (int)buf_size, fmt, args);
#else
    int w = vsnprintf(buf, buf_size, fmt, args);
#endif
    if (buf == NULL)
        return w;
    if (w == -1 || w >= (int)buf_size)
        w = (int)buf_size - 1;
    buf[w] = 0;
    return w;
}
#endif // #ifdef IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS

void ImFormatStringToTempBuffer(const char** out_buf, const char** out_buf_end, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    ImFormatStringToTempBufferV(out_buf, out_buf_end, fmt, args);
    va_end(args);
}

// FIXME: Should rework API toward allowing multiple in-flight temp buffers (easier and safer for caller)
// by making the caller acquire a temp buffer token, with either explicit or destructor release, e.g.
//  ImGuiTempBufferToken token;
//  ImFormatStringToTempBuffer(token, ...);
void ImFormatStringToTempBufferV(const char** out_buf, const char** out_buf_end, const char* fmt, va_list args)
{
    ImGuiContext& g = *GImGui;
    if (fmt[0] == '%' && fmt[1] == 's' && fmt[2] == 0)
    {
        const char* buf = va_arg(args, const char*); // Skip formatting when using "%s"
        if (buf == NULL)
            buf = "(null)";
        *out_buf = buf;
        if (out_buf_end) { *out_buf_end = buf + ImStrlen(buf); }
    }
    else if (fmt[0] == '%' && fmt[1] == '.' && fmt[2] == '*' && fmt[3] == 's' && fmt[4] == 0)
    {
        int buf_len = va_arg(args, int); // Skip formatting when using "%.*s"
        const char* buf = va_arg(args, const char*);
        if (buf == NULL)
        {
            buf = "(null)";
            buf_len = ImMin(buf_len, 6);
        }
        *out_buf = buf;
        *out_buf_end = buf + buf_len; // Disallow not passing 'out_buf_end' here. User is expected to use it.
    }
    else
    {
        int buf_len = ImFormatStringV(g.TempBuffer.Data, g.TempBuffer.Size, fmt, args);
        *out_buf = g.TempBuffer.Data;
        if (out_buf_end) { *out_buf_end = g.TempBuffer.Data + buf_len; }
    }
}

#ifndef IMGUI_ENABLE_SSE4_2_CRC
// CRC32 needs a 1KB lookup table (not cache friendly)
// Although the code to generate the table is simple and shorter than the table itself, using a const table allows us to easily:
// - avoid an unnecessary branch/memory tap, - keep the ImHashXXX functions usable by static constructors, - make it thread-safe.
static const ImU32 GCrc32LookupTable[256] =
{
#ifdef IMGUI_USE_LEGACY_CRC32_ADLER
    // Legacy CRC32-adler table used pre 1.91.6 (before 2024/11/27). Only use if you cannot afford invalidating old .ini data.
    0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,0x9E6495A3,0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,0x7EB17CBD,0xE7B82D07,0x90BF1D91,
    0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,0x6DDDE4EB,0xF4D4B551,0x83D385C7,0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,
    0x3B6E20C8,0x4C69105E,0xD56041E4,0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,0x35B5A8FA,0x42B2986C,0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,
    0x26D930AC,0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,0xB6662D3D,
    0x76DC4190,0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,0x9FBFE4A5,0xE8B8D433,0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,0x086D3D2D,0x91646C97,0xE6635C01,
    0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,0x65B0D9C6,0x12B7E950,0x8BBEB8EA,0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,
    0x4DB26158,0x3AB551CE,0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,0x4369E96A,0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,
    0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,0xCE61E49F,0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,0xB7BD5C3B,0xC0BA6CAD,
    0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,0xEAD54739,0x9DD277AF,0x04DB2615,0x73DC1683,0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,
    0xF00F9344,0x8708A3D2,0x1E01F268,0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,0xFED41B76,0x89D32BE0,0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,
    0xD6D6A3E8,0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,0x4669BE79,
    0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,0x220216B9,0x5505262F,0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,
    0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,0x95BF4A82,0xE2B87A14,0x7BB12BAE,0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,
    0x86D3D2D4,0xF1D4E242,0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,0x88085AE6,0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,
    0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,0x3E6E77DB,0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,0x47B2CF7F,0x30B5FFE9,
    0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,0xCDD70693,0x54DE5729,0x23D967BF,0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D,
#else
    // CRC32c table compatible with SSE 4.2 instructions
    0x00000000,0xF26B8303,0xE13B70F7,0x1350F3F4,0xC79A971F,0x35F1141C,0x26A1E7E8,0xD4CA64EB,0x8AD958CF,0x78B2DBCC,0x6BE22838,0x9989AB3B,0x4D43CFD0,0xBF284CD3,0xAC78BF27,0x5E133C24,
    0x105EC76F,0xE235446C,0xF165B798,0x030E349B,0xD7C45070,0x25AFD373,0x36FF2087,0xC494A384,0x9A879FA0,0x68EC1CA3,0x7BBCEF57,0x89D76C54,0x5D1D08BF,0xAF768BBC,0xBC267848,0x4E4DFB4B,
    0x20BD8EDE,0xD2D60DDD,0xC186FE29,0x33ED7D2A,0xE72719C1,0x154C9AC2,0x061C6936,0xF477EA35,0xAA64D611,0x580F5512,0x4B5FA6E6,0xB93425E5,0x6DFE410E,0x9F95C20D,0x8CC531F9,0x7EAEB2FA,
    0x30E349B1,0xC288CAB2,0xD1D83946,0x23B3BA45,0xF779DEAE,0x05125DAD,0x1642AE59,0xE4292D5A,0xBA3A117E,0x4851927D,0x5B016189,0xA96AE28A,0x7DA08661,0x8FCB0562,0x9C9BF696,0x6EF07595,
    0x417B1DBC,0xB3109EBF,0xA0406D4B,0x522BEE48,0x86E18AA3,0x748A09A0,0x67DAFA54,0x95B17957,0xCBA24573,0x39C9C670,0x2A993584,0xD8F2B687,0x0C38D26C,0xFE53516F,0xED03A29B,0x1F682198,
    0x5125DAD3,0xA34E59D0,0xB01EAA24,0x42752927,0x96BF4DCC,0x64D4CECF,0x77843D3B,0x85EFBE38,0xDBFC821C,0x2997011F,0x3AC7F2EB,0xC8AC71E8,0x1C661503,0xEE0D9600,0xFD5D65F4,0x0F36E6F7,
    0x61C69362,0x93AD1061,0x80FDE395,0x72966096,0xA65C047D,0x5437877E,0x4767748A,0xB50CF789,0xEB1FCBAD,0x197448AE,0x0A24BB5A,0xF84F3859,0x2C855CB2,0xDEEEDFB1,0xCDBE2C45,0x3FD5AF46,
    0x7198540D,0x83F3D70E,0x90A324FA,0x62C8A7F9,0xB602C312,0x44694011,0x5739B3E5,0xA55230E6,0xFB410CC2,0x092A8FC1,0x1A7A7C35,0xE811FF36,0x3CDB9BDD,0xCEB018DE,0xDDE0EB2A,0x2F8B6829,
    0x82F63B78,0x709DB87B,0x63CD4B8F,0x91A6C88C,0x456CAC67,0xB7072F64,0xA457DC90,0x563C5F93,0x082F63B7,0xFA44E0B4,0xE9141340,0x1B7F9043,0xCFB5F4A8,0x3DDE77AB,0x2E8E845F,0xDCE5075C,
    0x92A8FC17,0x60C37F14,0x73938CE0,0x81F80FE3,0x55326B08,0xA759E80B,0xB4091BFF,0x466298FC,0x1871A4D8,0xEA1A27DB,0xF94AD42F,0x0B21572C,0xDFEB33C7,0x2D80B0C4,0x3ED04330,0xCCBBC033,
    0xA24BB5A6,0x502036A5,0x4370C551,0xB11B4652,0x65D122B9,0x97BAA1BA,0x84EA524E,0x7681D14D,0x2892ED69,0xDAF96E6A,0xC9A99D9E,0x3BC21E9D,0xEF087A76,0x1D63F975,0x0E330A81,0xFC588982,
    0xB21572C9,0x407EF1CA,0x532E023E,0xA145813D,0x758FE5D6,0x87E466D5,0x94B49521,0x66DF1622,0x38CC2A06,0xCAA7A905,0xD9F75AF1,0x2B9CD9F2,0xFF56BD19,0x0D3D3E1A,0x1E6DCDEE,0xEC064EED,
    0xC38D26C4,0x31E6A5C7,0x22B65633,0xD0DDD530,0x0417B1DB,0xF67C32D8,0xE52CC12C,0x1747422F,0x49547E0B,0xBB3FFD08,0xA86F0EFC,0x5A048DFF,0x8ECEE914,0x7CA56A17,0x6FF599E3,0x9D9E1AE0,
    0xD3D3E1AB,0x21B862A8,0x32E8915C,0xC083125F,0x144976B4,0xE622F5B7,0xF5720643,0x07198540,0x590AB964,0xAB613A67,0xB831C993,0x4A5A4A90,0x9E902E7B,0x6CFBAD78,0x7FAB5E8C,0x8DC0DD8F,
    0xE330A81A,0x115B2B19,0x020BD8ED,0xF0605BEE,0x24AA3F05,0xD6C1BC06,0xC5914FF2,0x37FACCF1,0x69E9F0D5,0x9B8273D6,0x88D28022,0x7AB90321,0xAE7367CA,0x5C18E4C9,0x4F48173D,0xBD23943E,
    0xF36E6F75,0x0105EC76,0x12551F82,0xE03E9C81,0x34F4F86A,0xC69F7B69,0xD5CF889D,0x27A40B9E,0x79B737BA,0x8BDCB4B9,0x988C474D,0x6AE7C44E,0xBE2DA0A5,0x4C4623A6,0x5F16D052,0xAD7D5351
#endif
};
#endif

// Known size hash
// It is ok to call ImHashData on a string with known length but the ### operator won't be supported.
// FIXME-OPT: Replace with e.g. FNV1a hash? CRC32 pretty much randomly access 1KB. Need to do proper measurements.
ImGuiID ImHashData(const void* data_p, size_t data_size, ImGuiID seed)
{
    ImU32 crc = ~seed;
    const unsigned char* data = (const unsigned char*)data_p;
    const unsigned char *data_end = (const unsigned char*)data_p + data_size;
#ifndef IMGUI_ENABLE_SSE4_2_CRC
    const ImU32* crc32_lut = GCrc32LookupTable;
    while (data < data_end)
        crc = (crc >> 8) ^ crc32_lut[(crc & 0xFF) ^ *data++];
    return ~crc;
#else
    while (data + 4 <= data_end)
    {
        crc = _mm_crc32_u32(crc, *(ImU32*)data);
        data += 4;
    }
    while (data < data_end)
        crc = _mm_crc32_u8(crc, *data++);
    return ~crc;
#endif
}

// Zero-terminated string hash, with support for ### to reset back to seed value.
// e.g. "label###id" outputs the same hash as "id" (and "label" is generally displayed by the UI functions)
// FIXME-OPT: Replace with e.g. FNV1a hash? CRC32 pretty much randomly access 1KB. Need to do proper measurements.
ImGuiID ImHashStr(const char* data_p, size_t data_size, ImGuiID seed)
{
    seed = ~seed;
    ImU32 crc = seed;
    const unsigned char* data = (const unsigned char*)data_p;
#ifndef IMGUI_ENABLE_SSE4_2_CRC
    const ImU32* crc32_lut = GCrc32LookupTable;
#endif
    if (data_size != 0)
    {
        while (data_size-- > 0)
        {
            unsigned char c = *data++;
            if (c == '#' && data_size >= 2 && data[0] == '#' && data[1] == '#')
            {
                crc = seed;
                data += 2;
                data_size -= 2;
                continue;
            }
#ifndef IMGUI_ENABLE_SSE4_2_CRC
            crc = (crc >> 8) ^ crc32_lut[(crc & 0xFF) ^ c];
#else
            crc = _mm_crc32_u8(crc, c);
#endif
        }
    }
    else
    {
        while (unsigned char c = *data++)
        {
            if (c == '#' && data[0] == '#' && data[1] == '#')
            {
                crc = seed;
                data += 2;
                continue;
            }
#ifndef IMGUI_ENABLE_SSE4_2_CRC
            crc = (crc >> 8) ^ crc32_lut[(crc & 0xFF) ^ c];
#else
            crc = _mm_crc32_u8(crc, c);
#endif
        }
    }
    return ~crc;
}

// Skip to the "###" marker if any. We don't skip past to match the behavior of GetID()
// FIXME-OPT: This is not designed to be optimal. Use with care.
const char* ImHashSkipUncontributingPrefix(const char* label)
{
    const char* result = label;
    while (unsigned char c = *label++)
        if (c == '#' && label[0] == '#' && label[1] == '#')
            result = label + 2;
    return result;
}

//-----------------------------------------------------------------------------
// [SECTION] MISC HELPERS/UTILITIES (File functions)
//-----------------------------------------------------------------------------

// Default file functions
#ifndef IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS

ImFileHandle ImFileOpen(const char* filename, const char* mode)
{
#if defined(_WIN32) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS) && (defined(__MINGW32__) || (!defined(__CYGWIN__) && !defined(__GNUC__)))
    // We need a fopen() wrapper because MSVC/Windows fopen doesn't handle UTF-8 filenames.
    // Previously we used ImTextCountCharsFromUtf8/ImTextStrFromUtf8 here but we now need to support ImWchar16 and ImWchar32!
    const int filename_wsize = ::MultiByteToWideChar(CP_UTF8, 0, filename, -1, NULL, 0);
    const int mode_wsize = ::MultiByteToWideChar(CP_UTF8, 0, mode, -1, NULL, 0);

    // Use stack buffer if possible, otherwise heap buffer. Sizes include zero terminator.
    // We don't rely on current ImGuiContext as this is implied to be a helper function which doesn't depend on it (see #7314).
    wchar_t local_temp_stack[FILENAME_MAX];
    ImVector<wchar_t> local_temp_heap;
    if (filename_wsize + mode_wsize > IM_COUNTOF(local_temp_stack))
        local_temp_heap.resize(filename_wsize + mode_wsize);
    wchar_t* filename_wbuf = local_temp_heap.Data ? local_temp_heap.Data : local_temp_stack;
    wchar_t* mode_wbuf = filename_wbuf + filename_wsize;
    ::MultiByteToWideChar(CP_UTF8, 0, filename, -1, filename_wbuf, filename_wsize);
    ::MultiByteToWideChar(CP_UTF8, 0, mode, -1, mode_wbuf, mode_wsize);
    return ::_wfopen(filename_wbuf, mode_wbuf);
#else
    return fopen(filename, mode);
#endif
}

// We should in theory be using fseeko()/ftello() with off_t and _fseeki64()/_ftelli64() with __int64, waiting for the PR that does that in a very portable pre-C++11 zero-warnings way.
bool    ImFileClose(ImFileHandle f)     { return fclose(f) == 0; }
ImU64   ImFileGetSize(ImFileHandle f)   { long off = 0, sz = 0; return ((off = ftell(f)) != -1 && !fseek(f, 0, SEEK_END) && (sz = ftell(f)) != -1 && !fseek(f, off, SEEK_SET)) ? (ImU64)sz : (ImU64)-1; }
ImU64   ImFileRead(void* data, ImU64 sz, ImU64 count, ImFileHandle f)           { return fread(data, (size_t)sz, (size_t)count, f); }
ImU64   ImFileWrite(const void* data, ImU64 sz, ImU64 count, ImFileHandle f)    { return fwrite(data, (size_t)sz, (size_t)count, f); }
#endif // #ifndef IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS

// Helper: Load file content into memory
// Memory allocated with IM_ALLOC(), must be freed by user using IM_FREE() == ImGui::MemFree()
// This can't really be used with "rt" because fseek size won't match read size.
void*   ImFileLoadToMemory(const char* filename, const char* mode, size_t* out_file_size, int padding_bytes)
{
    IM_ASSERT(filename && mode);
    if (out_file_size)
        *out_file_size = 0;

    ImFileHandle f;
    if ((f = ImFileOpen(filename, mode)) == NULL)
        return NULL;

    size_t file_size = (size_t)ImFileGetSize(f);
    if (file_size == (size_t)-1)
    {
        ImFileClose(f);
        return NULL;
    }

    void* file_data = IM_ALLOC(file_size + padding_bytes);
    if (file_data == NULL)
    {
        ImFileClose(f);
        return NULL;
    }
    if (ImFileRead(file_data, 1, file_size, f) != file_size)
    {
        ImFileClose(f);
        IM_FREE(file_data);
        return NULL;
    }
    if (padding_bytes > 0)
        memset((void*)(((char*)file_data) + file_size), 0, (size_t)padding_bytes);

    ImFileClose(f);
    if (out_file_size)
        *out_file_size = file_size;

    return file_data;
}

//-----------------------------------------------------------------------------
// [SECTION] MISC HELPERS/UTILITIES (ImText* functions)
//-----------------------------------------------------------------------------

IM_MSVC_RUNTIME_CHECKS_OFF

// Convert UTF-8 to 32-bit character, process single character input.
// A nearly-branchless UTF-8 decoder, based on work of Christopher Wellons (https://github.com/skeeto/branchless-utf8).
// We handle UTF-8 decoding error by skipping forward.
int ImTextCharFromUtf8(unsigned int* out_char, const char* in_text, const char* in_text_end)
{
    static const char lengths[32] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0 };
    static const int masks[]  = { 0x00, 0x7f, 0x1f, 0x0f, 0x07 };
    static const uint32_t mins[] = { 0x400000, 0, 0x80, 0x800, 0x10000 };
    static const int shiftc[] = { 0, 18, 12, 6, 0 };
    static const int shifte[] = { 0, 6, 4, 2, 0 };
    int len = lengths[*(const unsigned char*)in_text >> 3];
    int wanted = len + (len ? 0 : 1);

    // IMPORTANT: if in_text_end == NULL it assume we have enough space!
    if (in_text_end == NULL)
        in_text_end = in_text + wanted; // Max length, nulls will be taken into account.

    // Copy at most 'len' bytes, stop copying at 0 or past in_text_end. Branch predictor does a good job here,
    // so it is fast even with excessive branching.
    unsigned char s[4];
    s[0] = in_text + 0 < in_text_end ? in_text[0] : 0;
    s[1] = in_text + 1 < in_text_end ? in_text[1] : 0;
    s[2] = in_text + 2 < in_text_end ? in_text[2] : 0;
    s[3] = in_text + 3 < in_text_end ? in_text[3] : 0;

    // Assume a four-byte character and load four bytes. Unused bits are shifted out.
    *out_char  = (uint32_t)(s[0] & masks[len]) << 18;
    *out_char |= (uint32_t)(s[1] & 0x3f) << 12;
    *out_char |= (uint32_t)(s[2] & 0x3f) <<  6;
    *out_char |= (uint32_t)(s[3] & 0x3f) <<  0;
    *out_char >>= shiftc[len];

    // Accumulate the various error conditions.
    int e = 0;
    e  = (*out_char < mins[len]) << 6; // non-canonical encoding
    e |= ((*out_char >> 11) == 0x1b) << 7;  // surrogate half?
    e |= (*out_char > IM_UNICODE_CODEPOINT_MAX) << 8;  // out of range we can store in ImWchar (FIXME: May evolve)
    e |= (s[1] & 0xc0) >> 2;
    e |= (s[2] & 0xc0) >> 4;
    e |= (s[3]       ) >> 6;
    e ^= 0x2a; // top two bits of each tail byte correct?
    e >>= shifte[len];

    if (e)
    {
        // No bytes are consumed when *in_text == 0 || in_text == in_text_end.
        // One byte is consumed in case of invalid first byte of in_text.
        // All available bytes (at most `len` bytes) are consumed on incomplete/invalid second to last bytes.
        // Invalid or incomplete input may consume less bytes than wanted, therefore every byte has to be inspected in s.
        wanted = ImMin(wanted, !!s[0] + !!s[1] + !!s[2] + !!s[3]);
        *out_char = IM_UNICODE_CODEPOINT_INVALID;
    }

    return wanted;
}

int ImTextStrFromUtf8(ImWchar* buf, int buf_size, const char* in_text, const char* in_text_end, const char** in_text_remaining)
{
    ImWchar* buf_out = buf;
    ImWchar* buf_end = buf + buf_size;
    while (buf_out < buf_end - 1 && (!in_text_end || in_text < in_text_end) && *in_text)
    {
        unsigned int c;
        in_text += ImTextCharFromUtf8(&c, in_text, in_text_end);
        *buf_out++ = (ImWchar)c;
    }
    *buf_out = 0;
    if (in_text_remaining)
        *in_text_remaining = in_text;
    return (int)(buf_out - buf);
}

int ImTextCountCharsFromUtf8(const char* in_text, const char* in_text_end)
{
    int char_count = 0;
    while ((!in_text_end || in_text < in_text_end) && *in_text)
    {
        unsigned int c;
        in_text += ImTextCharFromUtf8(&c, in_text, in_text_end);
        char_count++;
    }
    return char_count;
}

// Based on stb_to_utf8() from github.com/nothings/stb/
static inline int ImTextCharToUtf8_inline(char* buf, int buf_size, unsigned int c)
{
    if (c < 0x80)
    {
        buf[0] = (char)c;
        return 1;
    }
    if (c < 0x800)
    {
        if (buf_size < 2) return 0;
        buf[0] = (char)(0xc0 + (c >> 6));
        buf[1] = (char)(0x80 + (c & 0x3f));
        return 2;
    }
    if (c < 0x10000)
    {
        if (buf_size < 3) return 0;
        buf[0] = (char)(0xe0 + (c >> 12));
        buf[1] = (char)(0x80 + ((c >> 6) & 0x3f));
        buf[2] = (char)(0x80 + ((c ) & 0x3f));
        return 3;
    }
    if (c <= 0x10FFFF)
    {
        if (buf_size < 4) return 0;
        buf[0] = (char)(0xf0 + (c >> 18));
        buf[1] = (char)(0x80 + ((c >> 12) & 0x3f));
        buf[2] = (char)(0x80 + ((c >> 6) & 0x3f));
        buf[3] = (char)(0x80 + ((c ) & 0x3f));
        return 4;
    }
    // Invalid code point, the max unicode is 0x10FFFF
    return 0;
}

int ImTextCharToUtf8(char out_buf[5], unsigned int c)
{
    int count = ImTextCharToUtf8_inline(out_buf, 5, c);
    out_buf[count] = 0;
    return count;
}

// Not optimal but we very rarely use this function.
int ImTextCountUtf8BytesFromChar(const char* in_text, const char* in_text_end)
{
    unsigned int unused = 0;
    return ImTextCharFromUtf8(&unused, in_text, in_text_end);
}

static inline int ImTextCountUtf8BytesFromChar(unsigned int c)
{
    if (c < 0x80) return 1;
    if (c < 0x800) return 2;
    if (c < 0x10000) return 3;
    if (c <= 0x10FFFF) return 4;
    return 3;
}

int ImTextStrToUtf8(char* out_buf, int out_buf_size, const ImWchar* in_text, const ImWchar* in_text_end)
{
    char* buf_p = out_buf;
    const char* buf_end = out_buf + out_buf_size;
    while (buf_p < buf_end - 1 && (!in_text_end || in_text < in_text_end) && *in_text)
    {
        unsigned int c = (unsigned int)(*in_text++);
        if (c < 0x80)
            *buf_p++ = (char)c;
        else
            buf_p += ImTextCharToUtf8_inline(buf_p, (int)(buf_end - buf_p - 1), c);
    }
    *buf_p = 0;
    return (int)(buf_p - out_buf);
}

int ImTextCountUtf8BytesFromStr(const ImWchar* in_text, const ImWchar* in_text_end)
{
    int bytes_count = 0;
    while ((!in_text_end || in_text < in_text_end) && *in_text)
    {
        unsigned int c = (unsigned int)(*in_text++);
        if (c < 0x80)
            bytes_count++;
        else
            bytes_count += ImTextCountUtf8BytesFromChar(c);
    }
    return bytes_count;
}

const char* ImTextFindPreviousUtf8Codepoint(const char* in_text_start, const char* in_p)
{
    while (in_p > in_text_start)
    {
        in_p--;
        if ((*in_p & 0xC0) != 0x80)
            return in_p;
    }
    return in_text_start;
}

const char* ImTextFindValidUtf8CodepointEnd(const char* in_text_start, const char* in_text_end, const char* in_p)
{
    if (in_text_start == in_p)
        return in_text_start;
    const char* prev = ImTextFindPreviousUtf8Codepoint(in_text_start, in_p);
    unsigned int prev_c;
    int prev_c_len = ImTextCharFromUtf8(&prev_c, prev, in_text_end);
    if (prev_c != IM_UNICODE_CODEPOINT_INVALID && prev_c_len <= (int)(in_p - prev))
        return in_p;
    return prev;
}

int ImTextCountLines(const char* in_text, const char* in_text_end)
{
    if (in_text_end == NULL)
        in_text_end = in_text + ImStrlen(in_text); // FIXME-OPT: Not optimal approach, discourage use for now.
    int count = 0;
    while (in_text < in_text_end)
    {
        const char* line_end = (const char*)ImMemchr(in_text, '\n', in_text_end - in_text);
        in_text = line_end ? line_end + 1 : in_text_end;
        count++;
    }
    return count;
}

IM_MSVC_RUNTIME_CHECKS_RESTORE

//-----------------------------------------------------------------------------
// [SECTION] MISC HELPERS/UTILITIES (Color functions)
// Note: The Convert functions are early design which are not consistent with other API.
//-----------------------------------------------------------------------------

IMGUI_API ImU32 ImAlphaBlendColors(ImU32 col_a, ImU32 col_b)
{
    float t = ((col_b >> IM_COL32_A_SHIFT) & 0xFF) / 255.f;
    int r = ImLerp((int)(col_a >> IM_COL32_R_SHIFT) & 0xFF, (int)(col_b >> IM_COL32_R_SHIFT) & 0xFF, t);
    int g = ImLerp((int)(col_a >> IM_COL32_G_SHIFT) & 0xFF, (int)(col_b >> IM_COL32_G_SHIFT) & 0xFF, t);
    int b = ImLerp((int)(col_a >> IM_COL32_B_SHIFT) & 0xFF, (int)(col_b >> IM_COL32_B_SHIFT) & 0xFF, t);
    return IM_COL32(r, g, b, 0xFF);
}

ImVec4 ImGui::ColorConvertU32ToFloat4(ImU32 in)
{
    float s = 1.0f / 255.0f;
    return ImVec4(
        ((in >> IM_COL32_R_SHIFT) & 0xFF) * s,
        ((in >> IM_COL32_G_SHIFT) & 0xFF) * s,
        ((in >> IM_COL32_B_SHIFT) & 0xFF) * s,
        ((in >> IM_COL32_A_SHIFT) & 0xFF) * s);
}

ImU32 ImGui::ColorConvertFloat4ToU32(const ImVec4& in)
{
    ImU32 out;
    out  = ((ImU32)IM_F32_TO_INT8_SAT(in.x)) << IM_COL32_R_SHIFT;
    out |= ((ImU32)IM_F32_TO_INT8_SAT(in.y)) << IM_COL32_G_SHIFT;
    out |= ((ImU32)IM_F32_TO_INT8_SAT(in.z)) << IM_COL32_B_SHIFT;
    out |= ((ImU32)IM_F32_TO_INT8_SAT(in.w)) << IM_COL32_A_SHIFT;
    return out;
}

// Convert rgb floats ([0-1],[0-1],[0-1]) to hsv floats ([0-1],[0-1],[0-1]), from Foley & van Dam p592
// Optimized http://lolengine.net/blog/2013/01/13/fast-rgb-to-hsv
void ImGui::ColorConvertRGBtoHSV(float r, float g, float b, float& out_h, float& out_s, float& out_v)
{
    float K = 0.f;
    if (g < b)
    {
        ImSwap(g, b);
        K = -1.f;
    }
    if (r < g)
    {
        ImSwap(r, g);
        K = -2.f / 6.f - K;
    }

    const float chroma = r - (g < b ? g : b);
    out_h = ImFabs(K + (g - b) / (6.f * chroma + 1e-20f));
    out_s = chroma / (r + 1e-20f);
    out_v = r;
}

// Convert hsv floats ([0-1],[0-1],[0-1]) to rgb floats ([0-1],[0-1],[0-1]), from Foley & van Dam p593
// also http://en.wikipedia.org/wiki/HSL_and_HSV
void ImGui::ColorConvertHSVtoRGB(float h, float s, float v, float& out_r, float& out_g, float& out_b)
{
    if (s == 0.0f)
    {
        // gray
        out_r = out_g = out_b = v;
        return;
    }

    h = ImFmod(h, 1.0f) / (60.0f / 360.0f);
    int   i = (int)h;
    float f = h - (float)i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));

    switch (i)
    {
    case 0: out_r = v; out_g = t; out_b = p; break;
    case 1: out_r = q; out_g = v; out_b = p; break;
    case 2: out_r = p; out_g = v; out_b = t; break;
    case 3: out_r = p; out_g = q; out_b = v; break;
    case 4: out_r = t; out_g = p; out_b = v; break;
    case 5: default: out_r = v; out_g = p; out_b = q; break;
    }
}

#endif // #ifndef IMGUI_DISABLE
