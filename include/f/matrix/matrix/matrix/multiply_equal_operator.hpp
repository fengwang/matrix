#ifndef MMULTIPLY_EQUAL_OPERATOR_HPP_INCLUDED_DOH9ASFDKLH49Y8AFKLUHSFDKJHFSD984F
#define MMULTIPLY_EQUAL_OPERATOR_HPP_INCLUDED_DOH9ASFDKLH49Y8AFKLUHSFDKJHFSD984F

namespace f
{
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_multiply_equal_operator
    {
        typedef Matrix                                                          zen_type;
        typedef crtp_typedef<Type, Allocator>                                   type_proxy_type;
        typedef typename type_proxy_type::value_type                            value_type;
        typedef typename type_proxy_type::size_type                             size_type;
        typedef typename type_proxy_type::range_type                            range_type;

        zen_type& operator *=( const value_type& rhs )
        {
            zen_type& zen = static_cast<zen_type&>( *this );

            //std::transform( zen.begin(), zen.end(), zen.begin(), std::bind2nd( std::multiplies<value_type>(), rhs ) );
            for ( auto& v : zen )
                v *= rhs;

            return zen;
        }

#if 1

        /*
        ALGORITHM FOR:

                zen_type&
                operator *= ( const zen_type& other );
                with this[R] {C], other[C][OC]

                //ALGORITHM DETAILS:
                //threshold should be determined from experiments

                0 )  if ( ( max( all dims ) < threshold ) || min( all dims ) == 1 ) )
                    do direct multiplicaiton

                        //case row is odd
                        1 )  else if ( row() & 1 )
                    {
                        //<1>
                        if ( row() & 2 )
                            {
                                append one empty row to the matrix from downside
                                [ new_this < - this ]

                                do the multiplication
                                    [new_ans < - new_this * other ]
                                    remove the last row to generate ans
                                    [ ans < - new_ans ]
                                }

                            //<2>
                            else
                            {
                                cut last row from the matrix, get two new matrices
                                [ new_this < - this ]
                                [ last_row < - this ]

                                do the multiplications
                                    [ new_ans < - new_this * other ]
                                    [ last_row_ans < - last_row * other ]
                                    merge the two matrices to generate the ans
                                    [ ans < - new_ans | last_row_ans ]
                                    [ i.e. last_row_ans appended to new_ans as the last row ]
                                }
                        }

                //case col is odd
                        2 )  else if ( col() & 1 )
                    {
                        //<1>
                        if ( zen.col() & 2 )
                            {
                                append one emtpy col to this from right side
                                [new_this < - this]
                                append one empty row to other from downside
                                [new_other < - other]

                                do the multiplication
                                    [ans < - new_this * new_other]
                                }

                            //<2>
                            else
                            {
                                cut last col of this from right side
                                [new_this < - this]
                                [last_col < - this]
                                cut last row of other from downside
                                [new_other < - other]
                                [last_row < - other]

                                do the multiplicaitons
                                    [new_ans < - new_this * new_other]
                                    [res_col_row < - last_col * last_row]
                                    do the addition to generate ans
                                        [ans < - new_ans + res_col_row]
                                    }
                        }

                //case other.col is odd
                        3 )  else if ( other.col() & 1 )
                    {
                        //<1>
                        if ( other.col() & 2 )
                            {
                                append one empty col to other from right side
                                [new_other < - other]

                                do the multiplicaiton
                                    [new_ans = this * other]
                                    remove the last col to generate the ans
                                    [ans < - new_ans]
                                }

                            //<2>
                            else
                            {
                                cut the last col from other get two new matrices
                                [new_other < - other]
                                [last_col < - other]

                                do the multications
                                    [new_ans < - this * new_other]
                                    [last_col_ans < - this * last_col]
                                    merge the two matrices
                                    [ans < - new_ans | last_col_ans]
                                    [i.e. last_col_ans as the last col of]
                                }
                        }

                //all dims even, using strassen algorithm
                        4 ) else
                        {
                            //strassen algorithm

                            ( a_00 a_01 )( b_00 b_01 )( c_00 c_01 )
                            ( ) * ( )  = ( )
                            ( a_10 a_11 )( b_10 b_11 )( c_10 c_11 )

                            Q_0 = ( a_00 + a_11 ) * ( b_00 + b_11 )
                            Q_1 = ( a_10 + a_11 ) * b_00
                            Q_2 = a_00 * ( b_01 - b_11 )
                            Q_3 = a_11 * ( -b_00 + b_10 )
                            Q_4 = ( a_00 + a_01 ) * b_11
                            Q_5 = ( -a_00 + a_10 ) * ( b_00 + b_10 )
                            Q_6 = ( a_01 - a_11 ) * ( b_10 + b_11 )

                            c_00 = Q_0 + Q_3 - Q_4 + Q_6
                            c_10 = Q_1 + Q_3
                            c_01 = Q_2 + Q_4
                            c_11 = Q_0 - Q_1 + Q_2 + Q_5
                        }
        */


        zen_type& direct_multiply( const zen_type& other )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            assert( zen.col() == other.row() && "direct_multiply: dimesion not match!" );
            zen_type tmp( zen.row(), other.col() );

            for ( size_type i = 0; i < tmp.row(); ++i )
                for ( size_type j = 0; j < tmp.col(); ++j )
                {
                    tmp[i][j] = std::inner_product( zen.row_begin( i ), zen.row_end( i ), other.col_begin( j ), value_type( 0 ) );
                }

            zen.clone( tmp, 0, zen.row(), 0, other.col() );
            return zen;
        }

        /*
                1 )  else if ( row() & 1 )
            {
                //<1>
                if ( row() & 2 )
                    {
                        append one empty row to the matrix from downside
                        [ new_this < - this ] [R + 1, C]

                        do the multiplication
                            [new_ans < - new_this * other ] [R + 1, OC]
                            remove the last row to generate ans
                            [ ans < - new_ans ]
                        }
        */
        zen_type& rr1( const zen_type& other )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            const zen_type& new_this = zen && value_type( 0 );
            const zen_type& new_ans = new_this * other;
            zen.clone( new_ans, 0, zen.row(), 0, other.col() );
            return zen;
        }
        /*
        1 )  else if ( row() & 1 )
        {
            //<1>
            if ( row() & 2 ) {...}

            //<2>
            else
            {
                cut last row from the matrix, get two new matrices
                [ new_this < - this ] [R - 1, C]
                [ last_row < - this ] [1, C]

                do the multiplications
                    [ new_ans < - new_this * other ] [R - 1, OC]
                    [ last_row_ans < - last_row * other ] [1, OC]
                    merge the two matrices to generate the ans
                    [ ans < - new_ans | last_row_ans ] [R, OC]
                    [ i.e. last_row_ans appended to new_ans as the last row ]
                }
        }
        */
        zen_type& rr2( const zen_type& other )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            const zen_type new_this( zen, range_type( 0, zen.row() - 1 ), range_type( 0, zen.col() ) );
            const zen_type last_row( zen, range_type( zen.row() - 1, zen.row() ), range_type( 0, zen.col() ) );
            const zen_type& new_ans = new_this * other;
            const zen_type& last_row_ans = last_row * other;
            const zen_type& ans = new_ans && last_row_ans;
            zen.clone( ans, 0, zen.row(), 0, other.col() );
            return zen;
        }
        /*
        //case col is odd
        2 )  else if ( col() & 1 )
        {
        //<1>
        if ( zen.col() & 2 )
            {
                append one emtpy col to this from right side
                [new_this < - this] [R, C + 1]
                append one empty row to other from downside
                [new_other < - other] [C + 1, OC]

                do the multiplication
                    [ans < - new_this * new_other] [R, OC]
                }

            //<2>
            else {...}
            */
        zen_type& cc1( const zen_type& other )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            const zen_type& new_this = zen || value_type( 0 );
            /*
                    zen_type new_other( zen.col()+1, other.col() );
                    std::copy( other.begin(), other.end(), new_other.begin() );
            */
            const zen_type& new_other = other && value_type( 0 );
            const zen_type& ans = new_this * new_other;
            zen.clone( ans, 0, zen.row(), 0, other.col() );
            return zen;
        }
        /*
        //case col is odd
        2 )  else if ( col() & 1 )
        {
            //<1>
            if ( zen.col() & 2 ) {...}

            //<2>
            else
            {
                cut last col of this from right side
                [new_this < - this] [R, C - 1]
                [last_col < - this] [R, 1]
                cut last row of other from downside
                [new_other < - other] [C - 1, OC]
                [last_row < - other]  [1, OC]

                do the multiplicaitons
                    [new_ans < - new_this * new_other]    [R, OC]
                    [res_col_row < - last_col * last_row] [R, OC]
                    do the addition to generate ans
                        [ans < - new_ans + res_col_row] [R, OC]
                    }
        */
        zen_type& cc2( const zen_type& other )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            //[new_this <- this] [R,C-1]
            const zen_type new_this( zen, range_type( 0, zen.row() ), range_type( 0, zen.col() - 1 ) );
            //[last_col <- this] [R,1]
            const zen_type last_col( zen, range_type( 0, zen.row() ), range_type( zen.col() - 1, zen.col() ) );
            //[new_other <- other] [C-1,OC]
            const zen_type new_other( other, range_type( 0, other.row() - 1 ), range_type( 0, other.col() ) );
            //[last_row <- other]  [1, OC]
            const zen_type last_row( other, range_type( other.row() - 1, other.row() ), range_type( 0, other.col() ) );
            const zen_type& new_ans = new_this * new_other;
            const zen_type& res_col_row = last_col * last_row;
            const zen_type& ans = new_ans + res_col_row;
            zen.clone( ans, 0, zen.row(), 0, other.col() );
            return zen;
        }
        /*
        //case other.col is odd
        3 )  else if ( other.col() & 1 )
        {
            //<1>
            if ( other.col() & 2 )
            {
                append one empty col to other from right side
                [new_other < - other] [C, OC + 1]

                do the multiplicaiton
                    [new_ans = this * new_other] [R, OC + 1]
                    remove the last col to generate the ans
                    [ans < - new_ans] [R, OC]
                }

            //<2>
            else {...}
        }
        */
        zen_type& oc1( const zen_type& other )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            //[new_other <- other] [C,OC+1]
            const zen_type& new_other = other || value_type( 0 );
            const zen_type& new_ans = zen * new_other;
            zen.clone( new_ans, 0, zen.row(), 0, other.col() );
            return zen;
        }
        /*
        //case other.col is odd
        3 )  else if ( other.col() & 1 )
        {
        //<1>
        if ( other.col() & 2 ) {...}

            //<2>
            else
            {
                cut the last col from other get two new matrices
                [new_other < - other] [C, OC - 1]
                [last_col < - other]  [C, 1]

                do the multications
                    [new_ans < - this * new_other]     [R, OC - 1]
                    [last_col_ans < - this * last_col] [R, 1]
                    merge the two matrices
                    [ans < - new_ans | last_col_ans] [R, 1]
                    [i.e. last_col_ans as the last col of ans]
                }
        }
        */
        zen_type& oc2( const zen_type& other )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            const zen_type new_other( other, range_type( 0, other.row() ), range_type( 0, other.col() - 1 ) );
            const zen_type last_col( other, range_type( 0, other.row() ), range_type( other.col() - 1, other.col() ) );
            const zen_type& new_ans = zen * new_other;
            const zen_type& last_col_ans = zen * last_col;
            const zen_type& ans = new_ans || last_col_ans;
            zen.clone( ans, 0, zen.row(), 0, other.col() );
            return zen;
        }
        /*
        4 ) else
        {
            //strassen algorithm
        }
        */
        zen_type& strassen_multiply( const zen_type& other )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            const size_type R_2 = zen.row() >> 1;
            const size_type C_2 = zen.col() >> 1;
            const size_type OR_2 = C_2;
            const size_type OC_2 = other.col() >> 1;
            const zen_type a_00( zen, range_type( 0, R_2 ), range_type( 0, C_2 ) );
            const zen_type a_01( zen, range_type( 0, R_2 ), range_type( C_2, zen.col() ) );
            const zen_type a_10( zen, range_type( R_2, zen.row() ), range_type( 0, C_2 ) );
            const zen_type a_11( zen, range_type( R_2, zen.row() ), range_type( C_2, zen.col() ) );
            const zen_type b_00( other, range_type( 0, OR_2 ), range_type( 0, OC_2 ) );
            const zen_type b_01( other, range_type( 0, OR_2 ), range_type( OC_2, other.col() ) );
            const zen_type b_10( other, range_type( OR_2, other.row() ), range_type( 0, OC_2 ) );
            const zen_type b_11( other, range_type( OR_2, other.row() ), range_type( OC_2, other.col() ) );
            const zen_type& Q_0 = ( a_00 + a_11 ) * ( b_00 + b_11 );
            const zen_type& Q_1 = ( a_10 + a_11 ) * b_00;
            const zen_type& Q_2 = a_00 * ( b_01 - b_11 );
            const zen_type& Q_3 = a_11 * ( -b_00 + b_10 );
            const zen_type& Q_4 = ( a_00 + a_01 ) * b_11;
            const zen_type& Q_5 = ( -a_00 + a_10 ) * ( b_00 + b_01 );
            const zen_type& Q_6 = ( a_01 - a_11 ) * ( b_10 + b_11 );
            const zen_type& c_00 = Q_0 + Q_3 - Q_4 + Q_6;
            const zen_type& c_10 = Q_1 + Q_3;
            const zen_type& c_01 = Q_2 + Q_4;
            const zen_type& c_11 = Q_0 - Q_1 + Q_2 + Q_5;
            const zen_type& ans = ( c_00 || c_01 ) &&
                                  ( c_10 || c_11 );
            zen.clone( ans, 0, zen.row(), 0, other.col() );
            return zen;
        }

        zen_type& operator *= ( const zen_type& other )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            assert( zen.col() == other.row() );
            static const size_type threshold = 17;
            const size_type max_dims = std::max( std::max( zen.row(), zen.col() ), other.col() );
            const size_type min_dims = std::min( std::min( zen.row(), zen.col() ), other.col() );

            if ( ( max_dims < threshold )  || ( min_dims == 1 ) )
            {
                return direct_multiply( other );
            }

            const size_type R = zen.row();
            const size_type C = zen.col();
            const size_type OC = other.col();

            if ( R & 1 )
            {
                if ( R & 2 )
                {
                    return rr1( other );
                }

                return rr2( other );
            }

            if ( C & 1 )
            {
                if ( C & 2 )
                {
                    return cc1( other );
                }

                return cc2( other );
            }

            if ( OC & 1 )
            {
                if ( OC & 2 )
                {
                    return oc1( other );
                }

                return oc2( other );
            }

            return strassen_multiply( other );
        }
#endif
    };//struct
}
#endif//_MULTIPLY_EQUAL_OPERATOR_HPP_INCLUDED_DOH9ASFDKLH49Y8AFKLUHSFDKJHFSD984F
