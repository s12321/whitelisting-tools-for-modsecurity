/*
    This file is part of modsecurity-whitelister.

    modsecurity-whitelister is free software: you can redistribute it and/or
    modify it under the terms of the GNU General Public License as published
    by the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    modsecurity-whitelister is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with modsecurity-whitelister.  If not, see
    <http://www.gnu.org/licenses/>.
*/

#include "auditlogrecord.h"
#include <QString>
#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

// for apache log time to unix time function - rewrite to use Qt members
#include <sstream>
using std::string;
using std::stringstream;


// ============== AUDIT LOG RECORD ===============

void AuditLogRecord::importAuditLogHeader(QString A) {
    //qDebug() << "A";
    //qDebug().noquote() << A;

    //auditLogHeader = A;
    // TODO - use qsharedpointer instead of normal pointer for this?
    //delete auditLogHeader;
    auditLogHeader = new AuditLogHeader(A);
}

void AuditLogRecord::importRequestHeaders(QString B) {
    //qDebug() << "B";

    //requestHeaders = B;
    //delete requestHeaders;
    requestHeaders = new RequestHeaders(B);
}

void AuditLogRecord::importRequestBody(QString C) {
    //qDebug() << "C";
    requestBody = C;
}

void AuditLogRecord::importIntendedResponseHeaders(QString D) {
    //qDebug() << "D";
    intendedResponseHeaders = D;
}

void AuditLogRecord::importIntendedResponseBody(QString E) {
    //qDebug() << "E";
    intendedResponseBody = E;
}

void AuditLogRecord::importResponseHeaders(QString F) {
    //qDebug() << "F";
    responseHeaders = F;
}

void AuditLogRecord::importResponseBody(QString G) {
    //qDebug() << "G";
    responseBody = G;
}

void AuditLogRecord::importAuditLogTrailer(QString H) {
    //qDebug() << "H";
    auditLogTrailer = H;
}

void AuditLogRecord::importReducedMultipartRequestBody(QString I) {
    //qDebug() << "I";
    reducedMultipartRequestBody = I;
}

void AuditLogRecord::importMultipartFilesInformation(QString J) {
    //qDebug() << "J";
    multipartFilesInformation = J;
}

void AuditLogRecord::importMatchedRules(QString K) {
    //qDebug() << "K";
    matchedRules = K;
}

void AuditLogRecord::clear() {
    //qDebug() << "clearing record";

    //auditLogHeader.clear(); // A
    //requestHeaders.clear(); // B
    requestBody.clear(); // C
    intendedResponseHeaders.clear(); // D
    intendedResponseBody.clear(); // E
    responseHeaders.clear(); // F
    responseBody.clear(); // G
    auditLogTrailer.clear(); // H
    reducedMultipartRequestBody.clear(); // I
    multipartFilesInformation.clear(); // J
    matchedRules.clear(); // K

    auditLogHeader->clear(); // A
    requestHeaders->clear(); // B
}


// ============== AUDIT LOG HEADER ===============
AuditLogHeader::AuditLogHeader (QString data) {
    completeString = data;

    QRegularExpression regex("^\\[(.*)\\]\\s(.{24})\\s(\\d+\\.\\d+\\.\\d+\\.\\d+|::1)\\s(\\d+)\\s(\\d+\\.\\d+\\.\\d+\\.\\d+|::1)\\s(\\d+).*");
    QRegularExpressionMatch matches = regex.match(data);
    if(matches.hasMatch()) {

        apacheTimestamp = matches.captured(1);
        uniqueID = matches.captured(2);
        sourceIP = matches.captured(3);
        sourcePort = matches.captured(4);
        destinationIP = matches.captured(5);
        destinationPort = matches.captured(6);
        unixtime = QString::fromStdString(apachetimeToUnixtime(apacheTimestamp.toStdString()) );

        //qDebug() << "timestamp is " << apacheTimestamp;
    }
};

void AuditLogHeader::clear () {
    completeString.clear();

    unixtime.clear();
    apacheTimestamp.clear();
    uniqueID.clear();
    sourceIP.clear();
    sourcePort.clear();
    destinationIP.clear();
    destinationPort.clear();
};


string AuditLogHeader::apachetimeToUnixtime(const string &timestamp) {

    // convert Apache log time to unix time using this function http://www.thejach.com/view/2012/7/apaches_common_log_format_datetime_converted_to_unix_timestamp_with_c
    //#include <string>

    /*
     * Parses apache logtime into tm, converts to time_t, and reformats to str.
     * logtime should be the format: day/month/year:hour:minute:second zone
     * day = 2*digit
     * month = 3*letter
     * year = 4*digit
     * hour = 2*digit
     * minute = 2*digit
     * second = 2*digit
     * zone = (`+' | `-') 4*digit
     *
     * e.g. 04/Apr/2012:10:37:29 -0500
     */

    struct tm tm;
    time_t t;
    if (strptime(timestamp.c_str(), "%d/%b/%Y:%H:%M:%S %Z", &tm) == NULL)
    return "-";

    tm.tm_isdst = 0; // Force dst off
    // Parse the timezone, the five digits start with the sign at idx 21.
    int hours = 10*(timestamp[22] - '0') + timestamp[23] - '0';
    int mins = 10*(timestamp[24] - '0') + timestamp[25] - '0';
    int off_secs = 60*60*hours + 60*mins;
    if (timestamp[21] == '-')
    off_secs *= -1;

    t = mktime(&tm);
    if (t == -1)
    return "-";
    t -= timezone; // Local timezone
    t += off_secs;

    string retval;
    stringstream stream;
    stream << t;
    stream >> retval;
    return retval;
}

// ============== REQUEST HEADERS ================

RequestHeaders::RequestHeaders (QString data) {
    completeString = data;

    QRegularExpression B_regex("^(\\w+)\\s(.*)\\s(HTTP\\/\\d\\.\\d)$",QRegularExpression::MultilineOption); // 1st match is request method, 2nd match is URI, 3rd match is HTTP version
    //QRegularExpression B_regex_host("^Host:(.*?)$");
    QRegularExpression B_regex_host("^Host:\\s*(.*?)$",QRegularExpression::MultilineOption);
    QRegularExpression B_regex_connection("^Connection:(.*?)$",QRegularExpression::MultilineOption);
    QRegularExpression B_regex_accept("^Accept:(.*?)$",QRegularExpression::MultilineOption);
    QRegularExpression B_regex_useragent("^User-Agent:(.*?)$",QRegularExpression::MultilineOption);
    QRegularExpression B_regex_DNT("^DNT:(.*?)$",QRegularExpression::MultilineOption);
    QRegularExpression B_regex_referrer("^Referer:(.*?)$",QRegularExpression::MultilineOption);
    QRegularExpression B_regex_accept_encoding("^Accept-Encoding:(.*?)$",QRegularExpression::MultilineOption);
    QRegularExpression B_regex_accept_language("^Accept-Language:(.*?)$",QRegularExpression::MultilineOption);
    QRegularExpression B_regex_cookie("^Cookie:(.*?)$",QRegularExpression::MultilineOption);
    QRegularExpression B_regex_x_requested_with("^X-Requested-With:(.*?)$",QRegularExpression::MultilineOption);
    QRegularExpression B_regex_content_type("^Content-Type:(.*?)$",QRegularExpression::MultilineOption);
    QRegularExpression B_regex_content_length("^Content-Length:(.*?)$",QRegularExpression::MultilineOption);
    QRegularExpression B_regex_proxy_connection("^Proxy-Connection:(.*?)$",QRegularExpression::MultilineOption);
    QRegularExpression B_regex_accept_charset("^Accept-Charset:(.*?)$",QRegularExpression::MultilineOption);
    QRegularExpression B_regex_UA_CPU("^UA-CPU:(.*?)$",QRegularExpression::MultilineOption);
    QRegularExpression B_regex_x_forwarded_for("^X-Forwarded-For:(.*?)$",QRegularExpression::MultilineOption);
    QRegularExpression B_regex_cache_control("^Cache-Control:(.*?)$",QRegularExpression::MultilineOption);
    QRegularExpression B_regex_via("^Via:(.*?)$",QRegularExpression::MultilineOption);
    QRegularExpression B_regex_if_modified_since("^If-Modified-Since:(.*?)$",QRegularExpression::MultilineOption);
    QRegularExpression B_regex_if_none_match("^If-None-Match:(.*?)$",QRegularExpression::MultilineOption);
    QRegularExpression B_regex_pragma("^Pragma:(.*?)$",QRegularExpression::MultilineOption);

    QRegularExpressionMatch matches = B_regex.match(data);
    if (matches.hasMatch()) {

        requestMethod = matches.captured(1);
        uri = matches.captured(2);
        httpVersion = matches.captured(3);

    }

    matches = B_regex_host.match(data);
    if(matches.hasMatch()) {
        host = matches.captured(1);
    }

    matches = B_regex_connection.match(data);
    if(matches.hasMatch()) {
        connection = matches.captured(1);
    }

    matches = B_regex_accept.match(data);
    if(matches.hasMatch()) {
        accept = matches.captured(1);
    }

    matches = B_regex_useragent.match(data);
    if(matches.hasMatch()) {
        userAgent = matches.captured(1);
    }

    matches = B_regex_DNT.match(data);
    if(matches.hasMatch()) {
        dnt = matches.captured(1);
    }

    matches = B_regex_referrer.match(data);
    if(matches.hasMatch()) {
        referrer = matches.captured(1);
    }

    matches = B_regex_accept_encoding.match(data);
    if(matches.hasMatch()) {
        acceptEncoding = matches.captured(1);
    }

    matches = B_regex_accept_language.match(data);
    if(matches.hasMatch()) {
        acceptLanguage = matches.captured(1);
    }

    matches = B_regex_cookie.match(data);
    if(matches.hasMatch()) {
        cookie = matches.captured(1);
    }

    matches = B_regex_x_requested_with.match(data);
    if(matches.hasMatch()) {
        xRequestedWith = matches.captured(1);
    }

    matches = B_regex_content_type.match(data);
    if(matches.hasMatch()) {
        contentType = matches.captured(1);
    }

    matches = B_regex_content_length.match(data);
    if(matches.hasMatch()) {
        contentLength = matches.captured(1);
    }

    matches = B_regex_proxy_connection.match(data);
    if(matches.hasMatch()) {
        proxyConnection = matches.captured(1);
    }

    matches = B_regex_accept_charset.match(data);
    if(matches.hasMatch()) {
        acceptCharset = matches.captured(1);
    }

    matches = B_regex_UA_CPU.match(data);
    if(matches.hasMatch()) {
        userAgentCPU = matches.captured(1);
    }

    matches = B_regex_x_forwarded_for.match(data);
    if(matches.hasMatch()) {
        xForwardedFor = matches.captured(1);
    }

    matches = B_regex_cache_control.match(data);
    if(matches.hasMatch()) {
        cacheControl = matches.captured(1);
    }

    matches = B_regex_via.match(data);
    if(matches.hasMatch()) {
        via = matches.captured(1);
    }

    matches = B_regex_if_modified_since.match(data);
    if(matches.hasMatch()) {
        ifModifiedSince = matches.captured(1);
    }

    matches = B_regex_if_none_match.match(data);
    if(matches.hasMatch()) {
        ifNoneMatch = matches.captured(1);
    }

    matches = B_regex_pragma.match(data);
    if(matches.hasMatch()) {
        pragma = matches.captured(1);
    }

}

void RequestHeaders::clear() {

    completeString.clear();

    requestMethod.clear();
    uri.clear();
    httpVersion.clear();
    host.clear();
    connection.clear();
    accept.clear();
    userAgent.clear();
    dnt.clear();
    referrer.clear();
    acceptEncoding.clear();
    acceptLanguage.clear();
    cookie.clear();
    xRequestedWith.clear();
    contentType.clear();
    contentLength.clear();
    proxyConnection.clear();
    acceptCharset.clear();
    userAgentCPU.clear();
    xForwardedFor.clear();
    cacheControl.clear();
    via.clear();
    ifModifiedSince.clear();
    ifNoneMatch.clear();
    pragma.clear();
}
