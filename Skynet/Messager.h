#pragma once

#include <set>
#include <functional>
#include <tuple>

template <typename T>
class MessageReporterBase;

template <typename T>
class MessageListener
{
public:
	MessageListener( MessageReporterBase<T> &handler )
		: m_handler( &handler )
	{
		m_handler->registerForMessage( this );
	}

	virtual ~MessageListener()
	{
		m_handler->unRegisterForMessage( this );
	}

	virtual void notify( const T & message ) = 0;

	MessageListener( const MessageListener & ) = delete;
	MessageListener & operator=( const MessageListener & ) = delete;

private:
	MessageReporterBase<T> *m_handler;
};

template <typename T>
class MessageListenerFunction : MessageListener<T>
{
public:
	MessageListenerFunction( MessageReporterBase<T> &handler, std::function<void( const T & )> function )
		: MessageListener( handler )
		, m_function( std::move( function ) )
	{
	}

	void notify( const T & message ) override
	{
		m_function( message );
	}

private:
	std::function<void(const T &)> m_function;
};

template <typename T>
class MessageReporterBase
{
public:
	MessageReporterBase() = default;
	MessageReporterBase( const MessageReporterBase & ) = delete;
	MessageReporterBase & operator=( const MessageReporterBase & ) = delete;

	void basePostMessage( const T & message )
	{
		for( auto & listener : m_listeners )
			listener->notify( message );
	}

private:
	friend class MessageListener<T>;

	std::set<MessageListener<T>*> m_listeners;

	void registerForMessage( MessageListener<T>* listener )
	{
		m_listeners.insert( listener );
	}

	void unRegisterForMessage( MessageListener<T>* listener )
	{
		m_listeners.erase( listener );
	}
};

template <typename... MESSSAGES>
class MessageReporter : public MessageReporterBase<MESSSAGES>...
{
public:
	template <typename T>
	void postMessage( const T & message )
	{
		static_assert(std::is_convertible<MessageReporter, MessageReporterBase<T>>::value, "Message cannot handle that message type");
		static_cast<MessageReporterBase<T>*>(this)->basePostMessage( message );
	}

	template <typename T>
	void postMessage()
	{
		static_assert(std::is_convertible<MessageReporter, MessageReporterBase<T>>::value, "Message cannot handle that message type");
		static_cast<MessageReporterBase<T>*>(this)->basePostMessage( T() );
	}
};