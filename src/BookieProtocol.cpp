#include "BookieProtocol.h"
#include <folly/Format.h>
#include <iostream>

using namespace folly;

std::ostream& operator<<(std::ostream& s, BookieOperation op) {
    switch (op) {
    case BookieOperation::AddEntry:
        s << "AddEntry";
        break;
    case BookieOperation::ReadEntry:
        s << "ReadEntry";
        break;
    case BookieOperation::Auth:
        s << "Auth";
        break;
    default:
        s << "Unknown bookie op (" << (int) op << ")";
        break;
    }

    return s;
}

std::ostream& operator<<(std::ostream& s, BookieError error) {
    switch (error) {
    case BookieError::OK:
        s << "OK";
        break;
    case BookieError::NoLedger:
        s << "NoLedger";
        break;
    case BookieError::NoEntry:
        s << "NoEntry";
        break;
    case BookieError::BadRequest:
        s << "BadRequest";
        break;
    case BookieError::IOError:
        s << "IOError";
        break;
    case BookieError::UnauthorizedAccesss:
        s << "UnauthorizedAccess";
        break;
    case BookieError::BadVersion:
        s << "BadVersion";
        break;
    case BookieError::Fenced:
        s << "Fenced";
        break;
    case BookieError::ReadOnly:
        s << "ReadOnly";
        break;
    case BookieError::TooManyRequests:
        s << "TooManyRequests";
        break;
    }

    return s;
}

std::ostream& operator<<(std::ostream& s, const Request& r) {
    s << "Request(" //
            << "version:" << (int) r.protocolVersion //
            << " opCode:" << r.opCode //
            << " ledgerId:" << r.ledgerId //
            << " entryId:" << r.entryId //
            << " flags:0x" << format("{0:04x}", r.flags) //
            << " data-len:" << (r.data ? r.data->length() : 0) //
            << ")";

    return s;
}

std::ostream& operator<<(std::ostream& s, const Response& r) {
    s << "Response(" //
            << "version:" << (int) r.protocolVersion //
            << " opCode:" << r.opCode //
            << " error:" << r.errorCode //
            << " ledgerId:" << r.ledgerId //
            << " entryId:" << r.entryId //
            << " data-len:" << (r.data ? r.data->length() : 0) //
            << ")";

    return s;
}

