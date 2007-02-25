//
// MailMessage.h
//
// $Id: //poco/1.3/Net/include/Poco/Net/MailMessage.h#1 $
//
// Library: Net
// Package: Mail
// Module:  MailMessage
//
// Definition of the MailMessage class.
//
// Copyright (c) 2005-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#ifndef Net_MailMessage_INCLUDED
#define Net_MailMessage_INCLUDED


#include "Poco/Net/Net.h"
#include "Poco/Net/MessageHeader.h"
#include "Poco/Net/MailRecipient.h"
#include "Poco/Timestamp.h"
#include <vector>


namespace Poco {
namespace Net {


class MediaType;
class PartSource;
class PartHandler;
class MultipartWriter;


class Net_API MailMessage: public MessageHeader
	/// This class represents an e-mail message for
	/// use with the SMTPClientSession and POPClientSession
	/// classes.
	///
	/// MailMessage supports both old-style plain text messages,
	/// as well as MIME multipart mail messages with attachments.
	///
	/// For multi-part messages, the following content transfer
	/// encodings are supported: 7bit, 8bit, quoted-printable
	/// and base64.
{
public:	
	typedef std::vector<MailRecipient> Recipients;
	
	enum ContentDisposition
	{
		CONTENT_INLINE,
		CONTENT_ATTACHMENT
	};
	
	enum ContentTransferEncoding
	{
		ENCODING_7BIT,
		ENCODING_8BIT,
		ENCODING_QUOTED_PRINTABLE,
		ENCODING_BASE64
	};

	MailMessage();
		/// Creates an empty MailMessage.

	virtual ~MailMessage();
		/// Destroys the MailMessage.
		
	void addRecipient(const MailRecipient& recipient);
		/// Adds a recipient for the message.
		
	const Recipients& recipients() const;
		/// Returns the recipients of the message.

	void setSubject(const std::string& subject);
		/// Sets the subject of the message.
		
	const std::string& getSubject() const;
		/// Returns the subject of the message.
		
	void setSender(const std::string& sender);
		/// Sets the sender of the message (which
		/// ends up in the From header field).
		
	const std::string& getSender() const;
		/// Returns the sender of the message (taken
		/// from the From header field).

	void setContent(const std::string& content, ContentTransferEncoding encoding = ENCODING_QUOTED_PRINTABLE);
		/// Sets the content of the mail message.
		///
		/// If the content transfer encoding is ENCODING_7BIT or
		/// ENCODING_8BIT, the content string must be formatted
		/// according to the rules of an internet email message.
		///
		/// The message will be sent as a single-part
		/// message.
	
	const std::string& getContent() const;
		/// Returns the content of the mail message.
		///
		/// A content will only be returned for single-part
		/// messages. The content of multi-part mail messages
		/// will be reported through the registered PartHandler.
		
	void setContentType(const std::string& mediaType);
		/// Sets the content type for the message.
		
	void setContentType(const MediaType& mediaType);
		/// Sets the content type for the message.
		
	const std::string& getContentType() const;
		/// Returns the content type for the message.

	void setDate(const Poco::Timestamp& dateTime);
		/// Sets the Date header to the given date/time value.
		
	Poco::Timestamp getDate() const;
		/// Returns the value of the Date header.

	bool isMultipart() const;
		/// Returns true iff the message is a multipart message.

	void addPart(const std::string& name, PartSource* pSource, ContentDisposition disposition, ContentTransferEncoding encoding); 
		/// Adds a part/attachment to the mail message.
		///
		/// The MailMessage takes ownership of the PartSource and deletes it
		/// when it is no longer needed.
		///
		/// The MailMessage will be converted to a multipart message
		/// if it is not already one.

	void addContent(PartSource* pSource, ContentTransferEncoding encoding = ENCODING_QUOTED_PRINTABLE);
		/// Adds a part to the mail message by calling
		/// addPart("", pSource, CONTENT_INLINE, encoding);
		
	void addAttachment(const std::string& name, PartSource* pSource, ContentTransferEncoding encoding = ENCODING_BASE64);
		/// Adds an attachment to the mail message by calling
		/// addPart(name, pSource, CONTENT_ATTACHMENT, encoding);

	void read(std::istream& istr, PartHandler& handler);
		/// Reads the MailMessage from the given input stream.
		///
		/// If the message has multiple parts, the parts
		/// are reported to the PartHandler. If the message
		/// is not a multi-part message, the content is stored
		/// in a string available by calling getContent().

	void read(std::istream& istr);
		/// Reads the MailMessage from the given input stream.
		///
		/// The raw message (including all MIME parts) is stored
		/// in a string and available by calling getContent().

	void write(std::ostream& ostr) const;
		/// Writes the mail message to the given output stream.

protected:
	struct Part
	{
		std::string             name;
		PartSource*             pSource;
		ContentDisposition      disposition;
		ContentTransferEncoding encoding;
	};
	typedef std::vector<Part> PartVec;

	void makeMultipart();
	void writeHeader(const MessageHeader& header, std::ostream& ostr) const;
	void writeMultipart(MessageHeader& header, std::ostream& ostr) const;
	void writePart(MultipartWriter& writer, const Part& part) const;
	void writeEncoded(std::istream& istr, std::ostream& ostr, ContentTransferEncoding encoding) const;
	void setRecipientHeaders(MessageHeader& headers) const;
	void readHeader(std::istream& istr);
	void readMultipart(std::istream& istr, PartHandler& handler);
	void readPart(std::istream& istr, const MessageHeader& header, PartHandler& handler);
	void handlePart(std::istream& istr, const MessageHeader& header, PartHandler& handler);
	static const std::string& contentTransferEncodingToString(ContentTransferEncoding encoding);
	static int lineLength(const std::string& str);
	static void appendRecipient(const MailRecipient& recipient, std::string& str);

	static const std::string HEADER_SUBJECT;
	static const std::string HEADER_FROM;
	static const std::string HEADER_TO;
	static const std::string HEADER_CC;
	static const std::string HEADER_BCC;
	static const std::string HEADER_DATE;
	static const std::string HEADER_CONTENT_TYPE;
	static const std::string HEADER_CONTENT_TRANSFER_ENCODING;
	static const std::string HEADER_CONTENT_DISPOSITION;
	static const std::string HEADER_MIME_VERSION;
	static const std::string EMPTY_HEADER;
	static const std::string TEXT_PLAIN;
	static const std::string CTE_7BIT;
	static const std::string CTE_8BIT;
	static const std::string CTE_QUOTED_PRINTABLE;
	static const std::string CTE_BASE64;

private:
	MailMessage(const MailMessage&);
	MailMessage& operator = (const MailMessage&);

	Recipients              _recipients;
	PartVec                 _parts;
	std::string             _content;
	ContentTransferEncoding _encoding;
};


//
// inlines
//
inline const MailMessage::Recipients& MailMessage::recipients() const
{
	return _recipients;
}


inline const std::string& MailMessage::getContent() const
{
	return _content;
}


} } // namespace Poco::Net


#endif // Net_MailMessage_INCLUDED
