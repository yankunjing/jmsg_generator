#include "jmsg_writer.h"
#include "jmsg_util.h"
#include "jmsg_field.h"
#include <string>

void JMsgWriter::writeFieldHeader(JMsgField* field)
{
    writeEncodedLength(field->m_id);
}

void JMsgWriter::writeFieldHeader(int fieldId)
{
    writeEncodedLength(fieldId);
}

void JMsgWriter::writeArrayHeader(JMsgField* field, int length)
{
    writeEncodedLength(field->m_id);
    writeEncodedLength(length);
}

void JMsgWriter::writeStringField(JMsgField* field, const std::string& str)
{
    writeEncodedLength(field->m_id);
    writeString(str);
}

void JMsgWriter::writeWStringField(JMsgField * field, const std::wstring & str)
{
    writeEncodedLength(field->m_id);
    writeWString(str);
}

void JMsgWriter::writeIntField(JMsgField* field, const int value)
{
    writeEncodedLength(field->m_id);
    writeInt(value);
}

void JMsgWriter::writeInt64Field(JMsgField* field, const int64_t value)
{
    writeEncodedLength(field->m_id);
    writeInt64(value);
}

void JMsgWriter::writeDoubleField(JMsgField* field, const double value)
{
    writeEncodedLength(field->m_id);
    writeDouble(value);
}

void JMsgWriter::writeBoolField(JMsgField* field, const bool value)
{
    writeEncodedLength(field->m_id);
    writeBool(value);
}

void JMsgWriter::writeString(const std::string& str)
{
    unsigned char lenBuffer[4] = {};
    int encodedLen = jMsgEcodeSize((int)str.size(), lenBuffer);
    m_buffer.append((char*)lenBuffer, encodedLen);
    m_buffer.append(str.c_str(), str.size());
}

void JMsgWriter::writeWString(const std::wstring & str)
{
    unsigned char lenBuffer[4] = {};
    int encodedLen = jMsgEcodeSize((int)str.size() * 2, lenBuffer);
    m_buffer.append((char*)lenBuffer, encodedLen);
    m_buffer.append((char*)str.c_str(), str.size() *2);
}

void JMsgWriter::writeInt(const int value)
{
    unsigned char buf[4] = {};
    buf[0] = (value >> 24) & 0xff;
    buf[1] = (value >> 16) & 0xff;
    buf[2] = (value >> 8) & 0xff;
    buf[3] = value & 0xff;
    m_buffer.append((char*)buf, 4);
}

void JMsgWriter::writeInt64(const int64_t value)
{
    unsigned char buf[8] = {};
    buf[0] = (value >> 56) & 0xff;
    buf[1] = (value >> 48) & 0xff;
    buf[2] = (value >> 40) & 0xff;
    buf[3] = (value >> 32) & 0xff;
    buf[4] = (value >> 24) & 0xff;
    buf[5] = (value >> 16) & 0xff;
    buf[6] = (value >> 8) & 0xff;
    buf[7] = value & 0xff;
    m_buffer.append((char*)buf, 8);
}

void JMsgWriter::writeDouble(const double value)
{
    m_buffer.append((char*)&(value), sizeof(double));
}

void JMsgWriter::writeBool(const bool value)
{
    char val = value ? 1 : 0;
    m_buffer.append(&val, 1);
}

char* JMsgWriter::getBuffer()
{
    return (char*)m_buffer.c_str();
}

std::string& JMsgWriter::getBufferStr()
{
    return m_buffer;
}

int JMsgWriter::getBufferLen()
{
    return (int)m_buffer.size();
}

void JMsgWriter::writeEncodedLength(int len)
{
    unsigned char lenBuffer[4] = {};
    int encodedLen = jMsgEcodeSize(len, lenBuffer);
    m_buffer.append((char*)lenBuffer, encodedLen);
}

void JMsgWriter::appendBuffer(JMsgWriter* temp)
{
    m_buffer.append((char*)temp->getBuffer(), temp->getBufferLen());
}
