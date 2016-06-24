#pragma once

#include <folly/io/IOBuf.h>

#include <iosfwd>

using folly::IOBuf;

enum class BookieOperation
    : int8_t {
        /**
         * The Add entry request payload will be a ledger entry exactly as it should
         * be logged. The response payload will be a 4-byte integer that has the
         * error code followed by the 8-byte ledger number and 8-byte entry number
         * of the entry written.
         */
        AddEntry = 1,

        /**
         * The Read entry request payload will be the ledger number and entry number
         * to read. (The ledger number is an 8-byte integer and the entry number is
         * a 8-byte integer.) The response payload will be a 4-byte integer
         * representing an error code and a ledger entry if the error code is EOK,
         * otherwise it will be the 8-byte ledger number and the 4-byte entry number
         * requested. (Note that the first sixteen bytes of the entry happen to be
         * the ledger number and entry number as well.)
         */
        ReadEntry = 2,

        /**
         * Auth message. This code is for passing auth messages between the auth
         * providers on the client and bookie. The message payload is determined
         * by the auth providers themselves.
         */
        Auth = 3,
};

std::ostream& operator<<(std::ostream& s, BookieOperation op);

enum class BookieError
    : int8_t {
        /**
         * The error code that indicates success
         */
        OK = 0,

        /**
         * The error code that indicates that the ledger does not exist
         */
        NoLedger = 1,

        /**
         * The error code that indicates that the requested entry does not exist
         */
        NoEntry = 2,

        /**
         * The error code that indicates an invalid request type
         */
        BadRequest = 100,

        /**
         * General error occurred at the server
         */
        IOError = 101,

        /**
         * Unauthorized access to ledger
         */
        UnauthorizedAccesss = 102,

        /**
         * The server version is incompatible with the client
         */
        BadVersion = 103,

        /**
         * Attempt to write to fenced ledger
         */
        Fenced = 104,

        /**
         * The server is running as read-only mode
         */
        ReadOnly = 105,

        /**
         * Too many concurrent requests
         */
        TooManyRequests = 106,
};

std::ostream& operator<<(std::ostream& s, BookieError error);

enum class BookieFlag
    : int16_t {
        None = 0x0, DoFencing = 0x0001, Recovery = 0x0002,
};

struct BookieConstant {
    static const int64_t InvalidLedgerId = -1L;
    static const int64_t InvalidEntryId = -1L;
    static const uint32_t MasterKeyLength = 20;

    static constexpr uint32_t MaxFrameSize = 5 * 1024 * 1024;
};

typedef std::unique_ptr<IOBuf> IOBufPtr;

struct Request {
    int8_t protocolVersion;
    BookieOperation opCode;
    int64_t ledgerId;
    int64_t entryId;
    int16_t flags;

    IOBufPtr data;

    // Master key not supported
    // int8_t[] masterKey;

    bool isRecovery() const {
        return flags & (int16_t) BookieFlag::Recovery;
    }

    bool isFencing() const {
        return flags & (int16_t) BookieFlag::DoFencing;
    }
};

std::ostream& operator<<(std::ostream& s, const Request& request);

/////// Responses

struct Response {
    int8_t protocolVersion;
    BookieOperation opCode;
    BookieError errorCode;
    int64_t ledgerId;
    int64_t entryId;

    IOBufPtr data;
};

std::ostream& operator<<(std::ostream& s, const Response& response);
