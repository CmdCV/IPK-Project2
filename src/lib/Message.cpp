#include "../inc/Message.h"

#include <iostream>
#include <regex>
#include <stdexcept>
#include <sstream>

using namespace std;

// --- Statické validátory ---
void Message::validateLength(const string& value, size_t maxLength, const string& fieldName) {
    if (value.size() > maxLength) {
        throw invalid_argument(fieldName + " exceeds maximum length of " + to_string(maxLength));
    }
}

void Message::validateRegex(const string& value, const string& pattern, const string& fieldName) {
    if (!regex_match(value, regex(pattern))) {
//        throw invalid_argument(fieldName + " contains invalid characters.");
    }
}

// --- AuthMessage ---
AuthMessage::AuthMessage(const string& u, const string& d, const string& s)
    : Message(MessageType::AUTH), username(u), displayName(d), secret(s)
{
    validateLength(username, 20, "Username");
    validateRegex(username, "[a-zA-Z0-9_-]+", "Username");
    validateLength(displayName, 20, "DisplayName");
    validateRegex(displayName, "[\\x21-\\x7E]+", "DisplayName");
    validateLength(secret, 128, "Secret");
    validateRegex(secret, "[a-zA-Z0-9_-]+", "Secret");
}

string AuthMessage::serialize() const {
    return "AUTH " + username + " AS " + displayName + " USING " + secret + "\r\n";
}

vector<uint8_t> AuthMessage::serializeUDP(uint16_t msgId) const {
    vector<uint8_t> buf;
    buf.push_back(0x02);  // AUTH
    buf.push_back((msgId >> 8) & 0xFF); // MessageID
    buf.push_back(msgId & 0xFF);
    buf.insert(buf.end(), username.begin(), username.end());
    buf.push_back(0);
    buf.insert(buf.end(), displayName.begin(), displayName.end());
    buf.push_back(0);
    buf.insert(buf.end(), secret.begin(), secret.end());
    buf.push_back(0);
    return buf;
}

// --- JoinMessage ---
JoinMessage::JoinMessage(const string& c, const string& d)
    : Message(MessageType::JOIN), channelID(c), displayName(d)
{
    validateLength(channelID, 20, "ChannelID");
    validateRegex(channelID, "[a-zA-Z0-9_-]+", "ChannelID");
    validateLength(displayName, 20, "DisplayName");
    validateRegex(displayName, "[\\x21-\\x7E]+", "DisplayName");
}

string JoinMessage::serialize() const {
    return "JOIN " + channelID + " AS " + displayName + "\r\n";
}

vector<uint8_t> JoinMessage::serializeUDP(uint16_t msgId) const {
    vector<uint8_t> buf;
    buf.push_back(3);  // JOIN
    buf.push_back((msgId >> 8) & 0xFF);
    buf.push_back(msgId & 0xFF);
    buf.insert(buf.end(), channelID.begin(), channelID.end());
    buf.push_back(0);
    buf.insert(buf.end(), displayName.begin(), displayName.end());
    buf.push_back(0);
    return buf;
}

// --- MsgMessage ---
MsgMessage::MsgMessage(const string& d, const string& m)
    : Message(MessageType::MSG), displayName(d), messageContent(m)
{
    validateLength(displayName, 20, "DisplayName");
    validateRegex(displayName, "[\\x21-\\x7E]+", "DisplayName");
    validateLength(messageContent, 60000, "MessageContent");
    validateRegex(messageContent, "[\\x0A\\x20-\\x7E]+", "MessageContent");
}

string MsgMessage::serialize() const {
    return "MSG FROM " + displayName + " IS " + messageContent + "\r\n";
}

vector<uint8_t> MsgMessage::serializeUDP(uint16_t msgId) const {
    vector<uint8_t> buf;
    buf.push_back(4);  // MSG
    buf.push_back((msgId >> 8) & 0xFF);
    buf.push_back(msgId & 0xFF);
    buf.insert(buf.end(), displayName.begin(), displayName.end());
    buf.push_back(0);
    buf.insert(buf.end(), messageContent.begin(), messageContent.end());
    buf.push_back(0);
    return buf;
}

// --- ReplyMessage ---
ReplyMessage::ReplyMessage(bool s, const string& m, uint16_t r)
    : Message(MessageType::REPLY), success(s), messageContent(m), refMsgId(r)
{
    validateLength(messageContent, 60000, "MessageContent");
    validateRegex(messageContent, "[\\x0A\\x20-\\x7E]+", "MessageContent");
}

string ReplyMessage::serialize() const {
    string result = "REPLY ";
    result += (success ? "OK" : "NOK");
    result += " IS ";
    result += messageContent;
    result += "\r\n";
    return result;
}

vector<uint8_t> ReplyMessage::serializeUDP(uint16_t msgId) const {
    vector<uint8_t> buf;
    buf.push_back(0x01);                   // REPLY
    buf.push_back((msgId >> 8) & 0xFF);
    buf.push_back(msgId        & 0xFF);    // this reply’s own MessageID
    buf.push_back(success ? 1 : 0);        // Result
    buf.push_back((refMsgId >> 8) & 0xFF);
    buf.push_back(refMsgId        & 0xFF); // Ref_MessageID
    buf.insert(buf.end(), messageContent.begin(), messageContent.end());
    buf.push_back(0);                      // terminator
    return buf;
}

// --- ErrMessage ---
ErrMessage::ErrMessage(const string& d, const string& m)
    : Message(MessageType::ERR), displayName(d), messageContent(m)
{
    validateLength(displayName, 20, "DisplayName");
    validateRegex(displayName, "[\\x21-\\x7E]+", "DisplayName");
    validateLength(messageContent, 60000, "MessageContent");
    validateRegex(messageContent, "[\\x0A\\x20-\\x7E]+", "MessageContent");
}

string ErrMessage::serialize() const {
    return "ERR FROM " + displayName + " IS " + messageContent + "\r\n";
}

vector<uint8_t> ErrMessage::serializeUDP(uint16_t msgId) const {
    vector<uint8_t> buf;
    buf.push_back(5);  // ERR
    buf.push_back((msgId >> 8) & 0xFF);
    buf.push_back(msgId & 0xFF);
    buf.insert(buf.end(), displayName.begin(), displayName.end());
    buf.push_back(0);
    buf.insert(buf.end(), messageContent.begin(), messageContent.end());
    buf.push_back(0);
    return buf;
}

// --- ByeMessage ---
ByeMessage::ByeMessage(const string& d)
    : Message(MessageType::BYE), displayName(d)
{
    validateLength(displayName, 20, "DisplayName");
    validateRegex(displayName, "[\\x21-\\x7E]+", "DisplayName");
}

string ByeMessage::serialize() const {
    return "BYE FROM " + displayName + "\r\n";
}

vector<uint8_t> ByeMessage::serializeUDP(uint16_t msgId) const {
    vector<uint8_t> buf;
    buf.push_back(6);  // BYE
    buf.push_back((msgId >> 8) & 0xFF);
    buf.push_back(msgId & 0xFF);
    buf.insert(buf.end(), displayName.begin(), displayName.end());
    buf.push_back(0);
    return buf;
}

// --- ConfirmMessage ---
ConfirmMessage::ConfirmMessage()
    : Message(MessageType::CONFIRM) {}

string ConfirmMessage::serialize() const {
    return "CONFIRM\r\n";
}

vector<uint8_t> ConfirmMessage::serializeUDP(uint16_t msgId) const {
    vector<uint8_t> buf;
    buf.push_back(0);  // CONFIRM
    buf.push_back((msgId >> 8) & 0xFF);
    buf.push_back(msgId & 0xFF);
    return buf;
}

// --- PingMessage ---
PingMessage::PingMessage()
    : Message(MessageType::PING) {}

string PingMessage::serialize() const {
    return "PING\r\n";
}

vector<uint8_t> PingMessage::serializeUDP(uint16_t msgId) const {
    vector<uint8_t> buf;
    buf.push_back(253);  // PING
    buf.push_back((msgId >> 8) & 0xFF);
    buf.push_back(msgId & 0xFF);
    return buf;
}

// --- MessageFactory ---
unique_ptr<Message> MessageFactory::createMessage(MessageType type, const vector<string>& params) {
    switch (type) {
        case MessageType::AUTH:    return make_unique<AuthMessage>(params[0], params[1], params[2]);
        case MessageType::JOIN:    return make_unique<JoinMessage>(params[0], params[1]);
        case MessageType::MSG:     return make_unique<MsgMessage>(params[0], params[1]);
		case MessageType::REPLY: {
   		 	bool success = (params[0] == "true");
    		const string& msg     = params[1];
   		 	// parse the third parameter (a string) to an unsigned long, then cast down
    		uint16_t refMsgId = static_cast<uint16_t>(stoul(params[2]));
    		return make_unique<ReplyMessage>(success, msg, refMsgId);
		}
        case MessageType::ERR:     return make_unique<ErrMessage>(params[0], params[1]);
        case MessageType::BYE:     return make_unique<ByeMessage>(params[0]);
        case MessageType::CONFIRM: return make_unique<ConfirmMessage>();
        case MessageType::PING:    return make_unique<PingMessage>();
        default:                   throw invalid_argument("Unknown message type");
    }
}

unique_ptr<Message> MessageFactory::parseMessage(const string& input) {
    istringstream iss(input);
    string type;
    iss >> type;

    if (type == "AUTH") {
        string u, AS, d, USING, s;
        iss >> u >> AS >> d >> USING >> s;
        return createMessage(MessageType::AUTH, {u, s, d});
    } else if (type == "JOIN") {
        string c, AS, d;
        iss >> c >> AS >> d;
        return createMessage(MessageType::JOIN, {c, d});
    } else if (type == "MSG") {
        string F, d, IS;
        iss >> F >> d >> IS;
        string m; getline(iss, m);
        cout << d << ": " << m.substr(1) << endl << flush;
        return createMessage(MessageType::MSG, {d, m.substr(1)});
    } else if (type == "REPLY") {
        string ok, IS;
        iss >> ok >> IS;
        string m; getline(iss, m);
        cout << "Action " << (ok == "OK" ? "Success: " : "Failure: ") << m.substr(1) << endl << flush;
        return createMessage(MessageType::REPLY, {ok == "OK" ? "true" : "false", m.substr(1)});
    } else if (type == "ERR") {
        string F, d, IS;
        iss >> F >> d >> IS;
        string m; getline(iss, m);
        cout << "ERROR FROM " << d << ": " << m.substr(1) << endl << flush;
        return createMessage(MessageType::ERR, {d, m.substr(1)});
    } else if (type == "BYE") {
        string F, d;
        iss >> F >> d;
        return createMessage(MessageType::BYE, {d});
    } else if (type == "PING") {
        return createMessage(MessageType::PING, {});
    } else if (type == "CONFIRM") {
        return createMessage(MessageType::CONFIRM, {});
    } else {
        throw invalid_argument("Unknown message type: " + type);
    }
}

unique_ptr<Message> MessageFactory::parseUDP(const uint8_t* data, size_t length) {
    if (length < 3) {
        throw invalid_argument("UDP frame too short");
    }
    uint8_t typeCode = data[0];
    uint16_t msgId = (uint16_t(data[1]) << 8) | data[2];
    const uint8_t* ptr = data + 3;
    size_t remaining = length - 3;

    // pomocná lambda na čtení nulou ukončeného stringu
    auto readString = [&](string& out) {
        size_t i = 0;
        while (i < remaining && ptr[i] != 0) i++;
        if (i >= remaining) throw invalid_argument("Malformed UDP string");
        out.assign(reinterpret_cast<const char*>(ptr), i);
        ptr += i + 1;
        remaining -= i + 1;
    };

    switch (typeCode) {
        case 0x01: { // REPLY
            // Formát: [type][msgId][success][refMsgId(2B)][payload...]\0
            if (remaining < 3) throw invalid_argument("Malformed REPLY");
            bool success = (*ptr++ != 0);
            remaining--;
            // Read two-byte Ref_MessageID
            if (remaining < 2) throw invalid_argument("Malformed REPLY: missing Ref_MessageID");
            uint16_t refMsgId = (uint16_t(ptr[0]) << 8) | ptr[1];
            ptr += 2;
            remaining -= 2;
            string msg;
            readString(msg);
            cout << "Action " << (success ? "Success: " : "Failure: ") << msg << endl << flush;
            return make_unique<ReplyMessage>(success, msg, refMsgId);
        }

        case 0x00:  // CONFIRM
            return make_unique<ConfirmMessage>();

        case 0xFD: // PING
            return make_unique<PingMessage>();

        case 0xFE: { // ERR
            string dn, msg;
            readString(dn);
            readString(msg);
            cout << "ERROR FROM " << dn << ": " << msg << endl << flush;
            return make_unique<ErrMessage>(dn, msg);
        }

        case 0xFF: { // BYE (server‑initiated)
            string dn;
            readString(dn);
            // u BYE se neodpovídá CONFIRM, jen ukončíme
            return make_unique<ByeMessage>(dn);
        }

        case 0x04: { // MSG
            string dn, msg;
            readString(dn);
            readString(msg);
            cout << dn << ": " << msg << endl << flush;
            return make_unique<MsgMessage>(dn, msg);
        }

        case 0x03: { // JOIN
            string cid, dn;
            readString(cid);
            readString(dn);
            return make_unique<JoinMessage>(cid, dn);
        }

        default:
            throw invalid_argument("Unknown UDP message type code: " + to_string(typeCode));
    }
}
