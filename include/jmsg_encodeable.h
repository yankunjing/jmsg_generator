#ifndef JMSG_ENCODEABLE_H
#define JMSG_ENCODEABLE_H
#include "json/value.h"
class JMsgProto;
class JMsgWriter;
class JMsgReader;
class IJMsgEncodeable {
public:
    virtual void encode(JMsgWriter* writer) {}
    virtual bool decode(JMsgReader* reader) {return true;}
    virtual void encodeJson(Json::Value& val){}
    virtual bool decodeJson(Json::Value& val) {return true;}
    int getMsgId() { return m_msgId; }
protected:
    int m_msgId;
};
#endif
