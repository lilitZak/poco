//
// HTTPServerConnection.cpp
//
// $Id: //poco/1.3/Net/src/HTTPServerConnection.cpp#3 $
//
// Library: Net
// Package: HTTPServer
// Module:  HTTPServerConnection
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


#include "Poco/Net/HTTPServerConnection.h"
#include "Poco/Net/HTTPServerSession.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/NetException.h"
#include "Poco/NumberFormatter.h"
#include <memory>


namespace Poco {
namespace Net {


HTTPServerConnection::HTTPServerConnection(const StreamSocket& socket, HTTPServerParams* pParams, HTTPRequestHandlerFactory* pFactory):
	TCPServerConnection(socket),
	_pParams(pParams),
	_pFactory(pFactory)
{
	poco_check_ptr (pFactory);
	poco_check_ptr (pParams);
	
	_pParams->duplicate();
}


HTTPServerConnection::~HTTPServerConnection()
{
	_pParams->release();
}


void HTTPServerConnection::run()
{
	std::string server = _pParams->getSoftwareVersion();
	HTTPServerSession session(socket(), _pParams);
	while (session.hasMoreRequests())
	{
		try
		{
			HTTPServerRequest request(session, _pParams);
			HTTPServerResponse response(session);
			response.setVersion(request.getVersion());
			response.setKeepAlive(_pParams->getKeepAlive() && request.getKeepAlive() && session.canKeepAlive());
			if (!server.empty())
				response.set("Server", server);
			try
			{
				std::auto_ptr<HTTPRequestHandler> pHandler(_pFactory->createRequestHandler(request));
				if (pHandler.get())
				{
					if (request.expectContinue())
						response.sendContinue();
					
					pHandler->handleRequest(request, response);
					session.setKeepAlive(_pParams->getKeepAlive() && response.getKeepAlive() && session.canKeepAlive());
				}
				else sendErrorResponse(session, HTTPResponse::HTTP_NOT_IMPLEMENTED);
			}
			catch (Poco::Exception&)
			{
				if (!response.sent())
					sendErrorResponse(session, HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				throw;
			}
		}
		catch (NoMessageException&)
		{
			break;
		}
		catch (MessageException&)
		{
			sendErrorResponse(session, HTTPResponse::HTTP_BAD_REQUEST);
		}
	}
}


void HTTPServerConnection::sendErrorResponse(HTTPServerSession& session, HTTPResponse::HTTPStatus status)
{
	HTTPServerResponse response(session);
	response.setVersion(HTTPMessage::HTTP_1_1);
	response.setStatusAndReason(status);
	response.setKeepAlive(false);
	response.send();
	session.setKeepAlive(false);
}


} } // namespace Poco::Net
