#include "../inc/Message.h"

void Message::validateLength(const string& value, size_t maxLength, const string& fieldName) {
    if (value.length() > maxLength) {
        throw invalid_argument(fieldName + " exceeds maximum length of " + to_string(maxLength));
    }
}

void Message::validateRegex(const string& value, const string& pattern, const string& fieldName) {
    if (!regex_match(value, regex(pattern))) {
        throw invalid_argument(fieldName + " contains invalid characters.");
    }
}

AuthMessage::AuthMessage(const string& username, const string& secret, const string& displayName)
    :Message(MessageType::AUTH) {
    validateLength(username, 20, "Username");
    validateRegex(username, "[a-zA-Z0-9_-]+", "Username");
    validateLength(displayName, 20, "DisplayName");
    validateRegex(displayName, "[\\x21-\\x7E]+", "DisplayName");
    validateLength(secret, 128, "Secret");
    validateRegex(secret, "[a-zA-Z0-9_-]+", "Secret");

    this->username = username;
    this->displayName = displayName;
    this->secret = secret;
}

string AuthMessage::serialize() const {
    return "AUTH " + username + " " + displayName + " " + secret;
}

JoinMessage::JoinMessage(const string& channelID, const string& displayName)
    :Message(MessageType::JOIN) {
    validateLength(channelID, 20, "ChannelID");
    validateRegex(channelID, "[a-zA-Z0-9_-]+", "ChannelID");
    validateLength(displayName, 20, "DisplayName");
    validateRegex(displayName, "[\\x21-\\x7E]+", "DisplayName");

    this->channelID = channelID;
    this->displayName = displayName;
}

string JoinMessage::serialize() const {
    return "JOIN " + channelID + " " + displayName;
}

ErrMessage::ErrMessage(const string& displayName, const string& messageContent)
    :Message(MessageType::ERR) {
    validateLength(displayName, 20, "DisplayName");
    validateRegex(displayName, "[\\x21-\\x7E]+", "DisplayName");
    validateLength(messageContent, 60000, "MessageContent");
    validateRegex(messageContent, "[\\x0A\\x20-\\x7E]+", "MessageContent");

    this->displayName = displayName;
    this->messageContent = messageContent;
}

string ErrMessage::serialize() const {
    return "ERR " + displayName + " " + messageContent;
}

ByeMessage::ByeMessage(const string& displayName)
    :Message(MessageType::BYE) {
    validateLength(displayName, 20, "DisplayName");
    validateRegex(displayName, "[\\x21-\\x7E]+", "DisplayName");

    this->displayName = displayName;
}

string ByeMessage::serialize() const {
    return "BYE " + displayName;
}

MsgMessage::MsgMessage(const string& displayName, const string& messageContent)
    :Message(MessageType::MSG) {
    validateLength(displayName, 20, "DisplayName");
    validateRegex(displayName, "[\\x21-\\x7E]+", "DisplayName");
    validateLength(messageContent, 60000, "MessageContent");
    validateRegex(messageContent, "[\\x0A\\x20-\\x7E]+", "MessageContent");

    this->displayName = displayName;
    this->messageContent = messageContent;
}

string MsgMessage::serialize() const {
    return "MSG " + displayName + " " + messageContent;
}

ReplyMessage::ReplyMessage(bool success, const string& messageContent)
    :Message(MessageType::REPLY) {
    validateLength(messageContent, 60000, "MessageContent");
    validateRegex(messageContent, "[\\x0A\\x20-\\x7E]+", "MessageContent");

    this->success = success;
    this->messageContent = messageContent;
}

string ReplyMessage::serialize() const {
    return string("REPLY ") + (success ? "true" : "false") + " " + messageContent;
}

unique_ptr<Message> MessageFactory::createMessage(MessageType type, const vector<string>& params) {
    switch (type) {
        case MessageType::AUTH:
            if (params.size() != 3) throw invalid_argument("AUTH requires 3 parameters.");
            return make_unique<AuthMessage>(params[0], params[1], params[2]);
        case MessageType::JOIN:
            if (params.size() != 2) throw invalid_argument("JOIN requires 2 parameters.");
            return make_unique<JoinMessage>(params[0], params[1]);
        case MessageType::ERR:
            if (params.size() != 2) throw invalid_argument("ERR requires 2 parameters.");
            return make_unique<ErrMessage>(params[0], params[1]);
        case MessageType::BYE:
            if (params.size() != 1) throw invalid_argument("BYE requires 1 parameter.");
            return make_unique<ByeMessage>(params[0]);
        case MessageType::MSG:
            if (params.size() != 2) throw invalid_argument("MSG requires 2 parameters.");
            return make_unique<MsgMessage>(params[0], params[1]);
        case MessageType::REPLY:
            if (params.size() != 2) throw invalid_argument("REPLY requires 2 parameters.");
            return make_unique<ReplyMessage>(params[0] == "true", params[1]);
        default:
            throw invalid_argument("Unknown message type.");
    }
}
