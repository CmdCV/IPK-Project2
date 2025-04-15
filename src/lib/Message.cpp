#include "../inc/Message.h"

void Message::validateLength(const string& value, size_t maxLength, const string& fieldName) {
    if (value.length() > maxLength) {
        throw invalid_argument(fieldName + " exceeds maximum length of " + to_string(maxLength));
    }
}

void Message::validateRegex(const string& value, const string& pattern, const string& fieldName) {
    if (!regex_match(value, regex(pattern))) {
//        throw invalid_argument(fieldName + " contains invalid characters.");
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
    // AUTH {Username} AS {DisplayName} USING {Secret}\r\n
    return "AUTH " + username + " AS " + displayName + " USING " + secret + "\r\n";
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
    // JOIN {ChannelID} AS {DisplayName}\r\n
    return "JOIN " + channelID + " AS " + displayName+"\r\n";
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
    // ERR FROM {DisplayName} IS {MessageContent}\r\n
    return "ERR FROM " + displayName + " IS " + messageContent + "\r\n";
}

ByeMessage::ByeMessage(const string& displayName)
    :Message(MessageType::BYE) {
    validateLength(displayName, 20, "DisplayName");
    validateRegex(displayName, "[\\x21-\\x7E]+", "DisplayName");

    this->displayName = displayName;
}

string ByeMessage::serialize() const {
    // BYE FROM {DisplayName}\r\n
    return "BYE FROM " + displayName + "\r\n";
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
    // MSG FROM {DisplayName} IS {MessageContent}\r\n
    return "MSG FROM " + displayName + " IS " + messageContent + "\r\n";
}

ReplyMessage::ReplyMessage(bool success, const string& messageContent)
    :Message(MessageType::REPLY) {
    validateLength(messageContent, 60000, "MessageContent");
    validateRegex(messageContent, "[\\x0A\\x20-\\x7E]+", "MessageContent");

    this->success = success;
    this->messageContent = messageContent;
}

string ReplyMessage::serialize() const {
    // REPLY {"OK"|"NOK"} IS {MessageContent}\r\n
    return string("REPLY ") + (success ? "OK" : "NOK") + " IS " + messageContent + "\r\n";
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

unique_ptr<Message> MessageFactory::parseMessage(const string& input) {
    istringstream iss(input);
    string type;
    iss >> type;

    if (type == "AUTH") {
        string username, asKeyword, displayName, usingKeyword, secret;
        iss >> username >> asKeyword >> displayName >> usingKeyword >> secret;
        if (asKeyword != "AS" || usingKeyword != "USING") {
            throw invalid_argument("Invalid AUTH message format.");
        }
        vector<string> params;
        params.push_back(username);
        params.push_back(displayName);
        params.push_back(secret);
        return MessageFactory::createMessage(MessageType::AUTH, params);

    } else if (type == "JOIN") {
        string channelID, asKeyword, displayName;
        iss >> channelID >> asKeyword >> displayName;
        if (asKeyword != "AS") {
            throw invalid_argument("Invalid JOIN message format.");
        }
        vector<string> params;
        params.push_back(channelID);
        params.push_back(displayName);
        return MessageFactory::createMessage(MessageType::JOIN, params);

    } else if (type == "MSG") {
        string fromKeyword, displayName, isKeyword;
        iss >> fromKeyword >> displayName >> isKeyword;
        if (fromKeyword != "FROM" || isKeyword != "IS") {
            throw invalid_argument("Invalid MSG message format.");
        }
        string messageContent;
        getline(iss, messageContent);
        vector<string> params;
        params.push_back(displayName);
        params.push_back(messageContent.substr(1));
        // {DisplayName}: {MessageContent}\n
        cout << displayName << ": " << messageContent.substr(1) << endl;
        return MessageFactory::createMessage(MessageType::MSG, params);

    } else if (type == "REPLY") {
        string successStr, isKeyword;
        iss >> successStr >> isKeyword;
        if (isKeyword != "IS") {
            throw invalid_argument("Invalid REPLY message format.");
        }
        string messageContent;
        getline(iss, messageContent);
        bool success = (successStr == "OK");
        vector<string> params;
        params.push_back(success ? "true" : "false");
        params.push_back(messageContent.substr(1));
        // Action Success: {MessageContent}\n
        // Action Failure: {MessageContent}\n
        cout << "Action " << (success ? "Success: " : "Failure: ") << messageContent.substr(1) << endl;
        return MessageFactory::createMessage(MessageType::REPLY, params);

    } else if (type == "ERR") {
        string fromKeyword, displayName, isKeyword;
        iss >> fromKeyword >> displayName >> isKeyword;
        if (fromKeyword != "FROM" || isKeyword != "IS") {
            throw invalid_argument("Invalid ERR message format.");
        }
        string messageContent;
        getline(iss, messageContent);
        vector<string> params;
        params.push_back(displayName);
        params.push_back(messageContent.substr(1));
        // ERROR FROM {DisplayName}: {MessageContent}\n
        cout << "ERROR FROM " << displayName << ": " << messageContent.substr(1) << endl;
        return MessageFactory::createMessage(MessageType::ERR, params);

    } else if (type == "BYE") {
        string fromKeyword, displayName;
        iss >> fromKeyword >> displayName;
        if (fromKeyword != "FROM") {
            throw invalid_argument("Invalid BYE message format.");
        }
        vector<string> params;
        params.push_back(displayName);
        return MessageFactory::createMessage(MessageType::BYE, params);

    } else {
        throw invalid_argument("Unknown message type: " + type);
    }
}
