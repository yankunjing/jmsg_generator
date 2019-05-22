#include "jmsg_code_writer.h"
#include <stdarg.h>
#include <string>
#define TABSIZE 4

bool JMSGCodeWriter::open(std::string& path)
{
    std::string newPath = path;
    m_file = fopen(newPath.c_str(), "w");
    return m_file ? true : false;
}

void JMSGCodeWriter::addIndent()
{
    m_indent ++;
}

void JMSGCodeWriter::removeIndent()
{
    if (m_indent > 0) {
        m_indent --;
    }
}

void JMSGCodeWriter::writeLine(const char* format,...)
{
    for (size_t i = 0; i < m_indent * TABSIZE; i++) {
        fprintf(m_file, " ");
    }

    va_list ap;
    va_start(ap, format);
    vfprintf(m_file, format, ap);
    va_end(ap);
    fprintf(m_file, "\n");
    fflush(m_file);
}

void JMSGCodeWriter::write(const char* format,...)
{
    for (size_t i = 0; i < m_indent * TABSIZE; i++) {
        fprintf(m_file, " ");
    }

    va_list ap;
    va_start(ap, format);
    vfprintf(m_file, format, ap);
    va_end(ap);
    fflush(m_file);
}
