#ifndef MESSAGE_H
#define MESSAGE_H

#include "debugPrint.h"

#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <regex>
#include <memory>
#include <sstream>

using namespace std;

enum class MessageType {
    AUTH,       // Used for client authentication (signing in) using a user-provided username, display name and password
    JOIN,       // Represents the client's request to join a chat channel by its identifier
    MSG,        // Contains user display name and a message for the joined channel
    REPLY,      // Contains positive/negative confirmation for certain requests
    ERR,        // Indicates an error occurred, resulting in graceful termination of communication
    BYE,        // Indicates that the conversation/connection is to be terminated
    CONFIRM,    // UDP only: Explicitly confirms the successful delivery of the message
    PING,       // UDP only: Aliveness check mechanism sent periodically by the server
};

class Message {
protected:
    MessageType type;
public:
    Message(MessageType t) : type(t) {}
    virtual ~Message() = default;

    // Pro TCP
    virtual string serialize() const = 0;
    // Pro UDP (binární rámec)
    virtual vector<uint8_t> serializeUDP(uint16_t msgId) const = 0;

    MessageType getType() const { return type; }

    static void validateLength(const string& value, size_t maxLength, const string& fieldName);
    static void validateRegex(const string& value, const string& pattern, const string& fieldName);
};

// --- Deklarace jednotlivých zpráv ---

class AuthMessage : public Message {
    string username, displayName, secret;
public:
    AuthMessage(const string& username, const string& displayName, const string& secret);
    string serialize() const override;
    vector<uint8_t> serializeUDP(uint16_t msgId) const override;
};

class JoinMessage : public Message {
    string channelID, displayName;
public:
    JoinMessage(const string& channelID, const string& displayName);
    string serialize() const override;
    vector<uint8_t> serializeUDP(uint16_t msgId) const override;
};

class MsgMessage : public Message {
    string displayName, messageContent;
public:
    MsgMessage(const string& displayName, const string& messageContent);
    string serialize() const override;
    vector<uint8_t> serializeUDP(uint16_t msgId) const override;
};

class ReplyMessage : public Message {
    bool success;
    string messageContent;
    uint16_t refMsgId;
public:
    ReplyMessage(bool success, const string& messageContent, uint16_t refMsgId);
    string serialize() const override;
    vector<uint8_t> serializeUDP(uint16_t msgId) const override;
};

class ErrMessage : public Message {
    string displayName, messageContent;
public:
    ErrMessage(const string& displayName, const string& messageContent);
    string serialize() const override;
    vector<uint8_t> serializeUDP(uint16_t msgId) const override;
};

class ByeMessage : public Message {
    string displayName;
public:
    ByeMessage(const string& displayName);
    string serialize() const override;
    vector<uint8_t> serializeUDP(uint16_t msgId) const override;
};

class ConfirmMessage : public Message {
public:
    ConfirmMessage();
    string serialize() const override;
    vector<uint8_t> serializeUDP(uint16_t msgId) const override;
};

class PingMessage : public Message {
public:
    PingMessage();
    string serialize() const override;
    vector<uint8_t> serializeUDP(uint16_t msgId) const override;
};

class MessageFactory {
public:
    /**
     * @brief Creates a Message instance based on the provided type and parameters.
     * @param MessageType type
     * @param vector<string>& params
         * MessageType.AUTH => [string& username, string& secret, string& displayName]
         * MessageType.JOIN => [string& channelID, string& displayName]
         * MessageType.ERR => [string& displayName, string& messageContent]
         * MessageType.BYE => [string& displayName]
         * MessageType.MSG => [string& displayName, string& messageContent]
         * MessageType.REPLY => [bool success, string& messageContent]
     */
    static unique_ptr<Message> createMessage(MessageType type, const vector<string>& params);
    static unique_ptr<Message> parseMessage(const string& input);
    static unique_ptr<Message> parseUDP(const uint8_t* data, size_t length);
};

#endif // MESSAGE_H
