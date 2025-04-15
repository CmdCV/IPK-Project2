#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <stdexcept>
#include <regex>
#include <memory>

using namespace std;

enum class MessageType {
    AUTH,       // Used for client authentication (signing in) using a user-provided username, display name and password
    BYE,        // Indicates that the conversation/connection is to be terminated
    CONFIRM,    // UDP only: Explicitly confirms the successful delivery of the message
    ERR,        // Indicates an error occurred, resulting in graceful termination of communication
    JOIN,       // Represents the client's request to join a chat channel by its identifier
    MSG,        // Contains user display name and a message for the joined channel
    PING,       // UDP only: Aliveness check mechanism sent periodically by the server
    REPLY       // Contains positive/negative confirmation for certain requests
};

class Message {
protected:
    MessageType type; // Typ zprávy

public:
    explicit Message(MessageType type) : type(type) {}
    virtual ~Message() = default;

    virtual string serialize() const = 0;

    MessageType getType() const { return type; }

    static void validateLength(const string& value, size_t maxLength, const string& fieldName);
    static void validateRegex(const string& value, const string& pattern, const string& fieldName);
};

class AuthMessage : public Message {
    string username, displayName, secret;
public:
    AuthMessage(const string& username, const string& displayName, const string& secret);
    string serialize() const override;
};

class JoinMessage : public Message {
    string channelID, displayName;
public:
    JoinMessage(const string& channelID, const string& displayName);
    string serialize() const override;
};

class ErrMessage : public Message {
    string displayName, messageContent;
public:
    ErrMessage(const string& displayName, const string& messageContent);
    string serialize() const override;
};

class ByeMessage : public Message {
    string displayName;
public:
    ByeMessage(const string& displayName);
    string serialize() const override;
};

class MsgMessage : public Message {
    string displayName, messageContent;
public:
    MsgMessage(const string& displayName, const string& messageContent);
    string serialize() const override;
};

class ReplyMessage : public Message {
    bool success;
    string messageContent;
public:
    ReplyMessage(bool success, const string& messageContent);
    string serialize() const override;
};

/**
 * @brief Factory class for creating Message instances
 */
class MessageFactory {
public:
    /**
     * @brief Creates a Message instance based on the provided type and parameters.
     * @param MessageType type
     * @param vector<string>& params
         * MessageType.AUTH => [string& username, string& displayName, string& secret]
         * MessageType.JOIN => [string& channelID, string& displayName]
         * MessageType.ERR => [string& displayName, string& messageContent]
         * MessageType.BYE => [string& displayName]
         * MessageType.MSG => [string& displayName, string& messageContent]
         * MessageType.REPLY => [bool success, string& messageContent]
     */
    static unique_ptr<Message> createMessage(MessageType type, const vector<string>& params);
};

#endif //MESSAGE_H
