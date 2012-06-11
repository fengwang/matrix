#ifndef _SINGLETON_HPP_INCLUDED_ODISFJ948ILDFJOIUIRFGDUISOIURKLJFLKJASLDKJOIUSDLKJSALKFJEOIUJSODIFUEROIUSFDLKJROIUSFDLKJF
#define _SINGLETON_HPP_INCLUDED_ODISFJ948ILDFJOIUIRFGDUISOIURKLJFLKJASLDKJOIUSDLKJSALKFJEOIUJSODIFUEROIUSFDLKJROIUSFDLKJF

namespace feng
{
    template< typename T >
    struct singleton
    {
            typedef T value_type;
            typedef singleton self_type;

            singleton( const self_type& ) = delete;
            self_type& operator = ( const self_type& ) = delete;
            singleton( self_type&& ) = delete;
            self_type& operator = ( self_type&& ) = delete;
            singleton() = delete;

        private:
            struct constuctor
            {
                constuctor() { self_type::instance(); }
                inline void null_action() const { }
            };

            static constuctor constuctor_;

        public:
            static value_type&
            instance()
            {
                static value_type instance_;
                constuctor_.null_action();
                return instance_;
            }
    };

    template<typename T>
    typename singleton<T>::constuctor singleton<T>::constuctor_;    

}//namespace feng

#endif//_SINGLETON_HPP_INCLUDED_ODISFJ948ILDFJOIUIRFGDUISOIURKLJFLKJASLDKJOIUSDLKJSALKFJEOIUJSODIFUEROIUSFDLKJROIUSFDLKJF

