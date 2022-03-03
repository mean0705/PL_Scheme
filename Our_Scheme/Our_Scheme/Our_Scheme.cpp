// project2.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
//


# include <iostream>
# include <stdio.h>
# include <cstdio>
# include <stdlib.h>
# include <string.h>
# include <cstring>
# include <vector>
# include <cstdlib>
# include <iomanip>
# include <sstream>
# include <string>

using namespace std ;



enum TokenType {
    LEFT_PAREN, RIGHT_PAREN, INT, STRING, NOTHING,
    DOT, FLOAT, NIL, T, QUOTE, SYMBOL, WHITE_SPACE
} ;

class TreePtr {
public:
    string m_content ;
    TreePtr* m_right ;
    TreePtr* m_left ;
    TreePtr() {
        m_right = NULL ;
        m_left = NULL ;
        m_content = "\0" ;
    } // TreePtr()
} ; // TreePtr()

class Special_Symbol {
public:
    string m_content ;
    TreePtr* m_root ;
    Special_Symbol() {
        m_root = NULL ;
        m_content = "\0" ;
    } // Special_Symbol()
} ; // Special_Symbol()

class Token {
public:
    int m_line ;
    int m_column ;
    int m_whichType ;
    string m_str ;
    string m_error_type ;
    Token() {
        m_line = 0 ;
        m_column = 0 ;
        m_whichType = 0 ;
        m_str = "\0" ;
        m_error_type = "\0" ;
    } // Token()
} ; // Token()

class Exception {
public:
    string m_error_message ;
    TreePtr* m_root ;
    Exception() {
        m_error_message = "throw what?" ;
    } // Exception()

    Exception( string error_message, TreePtr* root ) {
        m_root = root ;
        m_error_message = error_message ;
    } // Exception()

}; // Exception()

static int u_line = 1 ;
static int u_column = 0 ;
static int u_first_exp_line = 0 ;
static int u_first_exp_column = 0 ;
static int u_testNum = 0 ;
static vector<Special_Symbol> u_cluster_Defined_Symbol ;
static vector<Special_Symbol> u_cluster_Procedure_Symbol ;

TreePtr* DoCons( TreePtr* temp ) ;
TreePtr* EvaluateTree( TreePtr* root, int FirstDefine ) ;


bool IsWhiteSpace( char ch ) {
    if ( ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' )
        return true ;
    else
        return false ;
} // IsWhiteSpace()

bool IsSemicolon( char ch ) {
    if ( ch == ';' )
        return true ;
    else
        return false ;
} // IsSemicolon()

bool IsDoubleQuote( char ch ) {
    if ( ch == '"' ) return true ;
    else return false ;
} // IsDoubleQuote()

bool IsSingleQuote( char ch ) {
    if ( ch == '\'' ) return true ;
    else return false ;
} // IsSingleQuote()

bool IsInt( char ch ) {
    if ( ch >= '0' && ch <= '9' ) return true ;
    else return false ;
} // IsInt()

bool IsNil( string str ) {
    if ( str == "nil" || str == "#f" || str == "()" ) return true ;
    else return false ;
} // IsNil()

bool IsIntString( string str ) {
    int num_count = 0 ;
    int i = 0 ;
    if ( str[i] == '-' || str[i] == '+' ) i++ ;

    for ( ; i < str.size() ; i++ ) {
        if ( IsInt( str[i] ) ) num_count++ ;
        if ( !IsInt( str[i] ) ) return false ;
    } // for

    if ( num_count == 0 ) return false ;
    return true ;
} // IsIntString()

bool IsFloatString( string str ) {
    int i = 0 ;
    int dot_count = 0 ;
    int num_count = 0 ;
    if ( str[i] == '-' || str[i] == '+' ) i++ ;

    for ( ; i < str.size() ; i++ ) {
        if ( str[i] == '.' ) dot_count++ ;
        if ( IsInt( str[i] ) ) num_count++ ;
        if ( ( !IsInt( str[i] ) && str[i] != '.' ) || dot_count > 1 ) return false ;
    } // for

    if ( num_count == 0 ) return false ;
    return true ;
} // IsFloatString()

int WhichType( string str ) {
    int str_size = str.size() ;
    if ( str == "" ) return NOTHING ;
    else if ( str == "(" ) return LEFT_PAREN ;
    else if ( str == ")" ) return RIGHT_PAREN ;
    else if ( str == "." ) return DOT ;
    else if ( str == "'" ) return QUOTE ;
    else if ( ( str == "t" ) || str == "#t" ) return T ;
    else if ( str == "nil" || str == "#f" || str == "()" ) return NIL ;
    else if ( str[str.size() - 1] == '"' && str[0] == '"' ) return STRING;
    else if ( IsIntString( str ) ) return INT ;
    else if ( IsFloatString( str ) ) return FLOAT ;
    else return SYMBOL ;

} // WhichType()

bool IsATOM( Token token ) {
    if ( token.m_whichType == SYMBOL || token.m_whichType == INT || token.m_whichType == FLOAT ||
        token.m_whichType == STRING || token.m_whichType == NIL || token.m_whichType == T )
        return true ;
    else return false ;
} // IsATOM()

bool IsSExpFirst( Token token ) {
    if ( token.m_whichType == SYMBOL || token.m_whichType == INT || token.m_whichType == FLOAT ||
        token.m_whichType == STRING || token.m_whichType == NIL || token.m_whichType == T ||
        token.m_whichType == LEFT_PAREN || token.m_whichType == QUOTE )
        return true ;
    else return false ;
} // IsSExpFirst()

void InitializeTree( TreePtr* node ) {
    node->m_left = NULL ;
    node->m_right = NULL ;
    node->m_content = "\0" ;
} // InitializeTree()

void IncreaseLine() {
    u_line++ ;
    u_column = 0 ;
} // IncreaseLine()

Token GetToken() {
    char ch = '\0' ;
    char ch_front = '\0' ;
    char peek = '\0' ;
    bool isBreak = false ;
    bool isSemoc = false ;
    bool has_close_error = false ;
    bool has_end_error = false ;
    bool has_semi = false ;

    Token token ;
    string str = "\0" ;
    ch = cin.get() ;
    while ( IsWhiteSpace( ch ) || ch == EOF ) {
        u_column++ ;
        if ( ch == EOF ) {
            token.m_column = u_column ;
            token.m_line = u_line ;
            token.m_str = "" ;
            return token ;
        } // if
        else if ( ch == '\n' ) {
            IncreaseLine() ;
        } // else if

        ch = cin.get() ;
    } // while

    peek = cin.peek() ;
    while ( isBreak == false ) {
        if ( ch == EOF ) {
            token.m_column = u_column ;
            token.m_line = u_line ;
            token.m_str = "" ;
            return token ;
        } // if
        else if ( IsSemicolon( ch ) ) { // 是分號
            u_column++ ;
            str = str + ch ;
            ch = cin.get() ;
            while ( ch != '\n' && ch != EOF ) {   // \\\\\\\\\n
                u_column++ ;
                str = str + ch ;
                ch = cin.get() ;
            } // while

            if ( ch == '\n' )
                IncreaseLine() ;
            else if ( ch == EOF )
                has_end_error = true ;

            isSemoc = true ;
            isBreak = true ;
        } // if
        else if ( IsDoubleQuote( ch ) ) { // 是雙引號 STRING
            u_column++ ;
            str = str + ch ;
            ch = cin.get() ;
            while ( ( ch != '"' || ( ch == '"' && ch_front == '\\' ) ) && ch != '\n' && ch != EOF ) {
                u_column++ ;
                str = str + ch ;
                ch_front = ch ;
                ch = cin.get() ;
            } // while

            if ( ch == '\n' ) has_close_error = true ;
            else if ( ch == EOF ) has_end_error = true ;
            else {
                str = str + ch ;
                u_column++ ;
            } // else

            isBreak = true ;
        } // else if
        else if ( ch == '(' || ch == ')' || ch == '\'' ) {
            str = str + ch ;
            u_column++ ;
            isBreak = true ;
        } // else if
        else if ( ch == '.' && peek == '(' && str.size() == 0 ) {
            str = str + ch ;
            u_column++ ;
            isBreak = true ;
        } // else if
        else if ( peek == '(' || peek == ')' || peek == '\'' || peek == '"' ) {
            str = str + ch ;
            u_column++ ;
            isBreak = true ;
        } // else if 
        else if ( IsWhiteSpace( peek ) ) { // next char is white space
            if ( ch != ';' ) {
                str = str + ch ;
                u_column++ ;
            } // if

            isBreak = true ;
        } // else if
        else if ( IsSemicolon( peek ) ) { // next char is 分號
            str = str + ch ;
            u_column++ ;
            isBreak = true ;
        } // else if
        else {
            str = str + ch ;
            u_column++ ;
            ch = cin.get() ;
            peek = cin.peek() ;
        } // else
    } // while


    if ( !isSemoc ) {
        if ( has_end_error == true ) {
            token.m_column = u_column ;
            token.m_line = u_line ;
            token.m_str = "" ;
            return token ;
        } // if

        token.m_str = str ;
        str.clear() ;
        token.m_whichType = WhichType( token.m_str ) ;

        if ( has_close_error == true ) {
            token.m_error_type = "error for close string" ;
            token.m_line = u_line ;
            token.m_column = u_column ;
            IncreaseLine() ;
        } // if
        else {
            token.m_line = u_line ;
            token.m_column = u_column - token.m_str.size() + 1 ;
        } // else
    } // if
    else {
        token = GetToken() ;
    } // else

    return token ;
} // GetToken()

void Collect_token( vector<Token>& cluster_token ) {
    Token token = GetToken() ;
    cluster_token.push_back( token ) ;
    while ( token.m_str != "\0" ) {
        token = GetToken() ;
        cluster_token.push_back( token ) ;
    } // while

} // Collect_token()

void Read_SExp( vector<Token>& cluster_token, vector<Token>& answer_cluster_token ) {
    // <S-exp> ::= <ATOM> 
    //           | LEFT - PAREN <S - exp> { <S - exp> } [DOT <S - exp>] RIGHT - PAREN //     (1{}[.2] )
    //           | QUOTE <S - exp>
    //  // 中括號: 可有可無 只能一次，  大括號: 可以無限次 可以沒有
    //    <ATOM>  :: = SYMBOL | INT | FLOAT | STRING
    //                | NIL | T | LEFT - PAREN RIGHT - PAREN
    Token answer_token ;

    if ( cluster_token[0].m_str == "" ) {
        cluster_token.erase( cluster_token.begin() ) ;
        throw "Error for no more input" ;
    } // if

    if ( cluster_token[0].m_error_type == "error for close string" ) {
        throw "error for close string" ;
    } // if

    if ( cluster_token[0].m_whichType == LEFT_PAREN ) { // (1) () atom  (2) syntax (3) exit
        if ( cluster_token[1].m_whichType == RIGHT_PAREN ) { // ()
            answer_token.m_whichType = NIL ;
            answer_token.m_str = "()" ;
            answer_token.m_line = cluster_token[1].m_line ;
            answer_token.m_column = cluster_token[1].m_column ;

            answer_cluster_token.push_back( answer_token ) ;
            cluster_token.erase( cluster_token.begin() ) ;
            cluster_token.erase( cluster_token.begin() ) ;
        } // if
        else if ( IsSExpFirst( cluster_token[1] ) ) { // syntax
            answer_cluster_token.push_back( cluster_token[0] ) ;
            cluster_token.erase( cluster_token.begin() ) ;

            Read_SExp( cluster_token, answer_cluster_token ) ;  // <S - exp>
            while ( IsSExpFirst( cluster_token[0] ) ) {         // { <S - exp> }
                Read_SExp( cluster_token, answer_cluster_token ) ;
            } // while

            if ( cluster_token[0].m_whichType == DOT ) {        // [DOT <S - exp>]
                if ( IsSExpFirst( cluster_token[1] ) ) {
                    answer_cluster_token.push_back( cluster_token[0] ) ;
                    cluster_token.erase( cluster_token.begin() ) ;
                    Read_SExp( cluster_token, answer_cluster_token ) ;
                } // if 
                else {
                    answer_cluster_token.push_back( cluster_token[0] ) ;
                    cluster_token.erase( cluster_token.begin() ) ;
                    throw "Error for syntax" ;
                } // else   
            } // if

            if ( cluster_token[0].m_whichType == RIGHT_PAREN ) { // RIGHT - PAREN
                answer_cluster_token.push_back( cluster_token[0] ) ;
                cluster_token.erase( cluster_token.begin() ) ;
            } // if
            else throw "Error for syntax fo right paren" ;
        } // else if
        else {
            answer_cluster_token.push_back( cluster_token[0] ) ;
            cluster_token.erase( cluster_token.begin() ) ;
            throw "Error for syntax" ;
        } // else
    } // if
    else if ( cluster_token[0].m_str == "'" ) { //  QUOTE <S - exp>
        answer_cluster_token.push_back( cluster_token[0] ) ;
        cluster_token.erase( cluster_token.begin() ) ;
        Read_SExp( cluster_token, answer_cluster_token ) ;
    } // else if
    else if ( IsATOM( cluster_token[0] ) ) {  // <ATOM>
        answer_cluster_token.push_back( cluster_token[0] ) ;
        cluster_token.erase( cluster_token.begin() ) ;
        return ;
    } // else if
    else {
        answer_cluster_token.push_back( cluster_token[0] ) ;
        throw "Error for syntax" ;
    } // else

} // Read_SExp()

TreePtr* CreateTree( vector<Token>& answer_cluster_token, TreePtr* node ) {
    TreePtr* temp = node ;
    if ( answer_cluster_token[0].m_str == "(" ) {
        answer_cluster_token.erase( answer_cluster_token.begin() ) ;
        node = new TreePtr ;
        InitializeTree( node ) ;
        node->m_left = CreateTree( answer_cluster_token, node->m_left ) ;

        temp = node ;
        while ( IsSExpFirst( answer_cluster_token[0] ) ) {         // { <S - exp> }
            temp->m_right = new TreePtr ;
            InitializeTree( temp->m_right ) ;

            temp = temp->m_right ;
            temp->m_left = CreateTree( answer_cluster_token, temp->m_left ) ;
        } // while

        if ( answer_cluster_token[0].m_whichType == DOT ) {        // [DOT <S - exp>]
            answer_cluster_token.erase( answer_cluster_token.begin() ) ;
            temp->m_right = CreateTree( answer_cluster_token, temp->m_right ) ;
        } // if

        if ( answer_cluster_token[0].m_whichType == RIGHT_PAREN ) { // RIGHT - PAREN
            answer_cluster_token.erase( answer_cluster_token.begin() ) ;
            if ( temp->m_content == "" && temp->m_right == NULL ) {
                temp->m_right = new TreePtr ;
                InitializeTree( temp->m_right ) ;
                temp->m_right->m_content = "nil" ;
            } // if

        } // if

        return node ;
    } // if
    else if ( answer_cluster_token[0].m_str == "'" ) {
        node = new TreePtr ;
        InitializeTree( node ) ;

        node->m_left = new TreePtr ;
        InitializeTree( node->m_left ) ;
        node->m_left->m_content = "quote" ;
        answer_cluster_token.erase( answer_cluster_token.begin() ) ;

        node->m_right = new TreePtr ;
        InitializeTree( node->m_right ) ;
        node->m_right->m_right = new TreePtr ;
        node->m_right->m_right->m_content = "nil" ;
        node->m_right->m_left = CreateTree( answer_cluster_token, node->m_right->m_left ) ;

        return node ;
    } // else if
    else if ( IsATOM( answer_cluster_token[0] ) ) {
        node = new TreePtr ;
        InitializeTree( node ) ;

        node->m_content = answer_cluster_token[0].m_str ;
        answer_cluster_token.erase( answer_cluster_token.begin() ) ;
        return node ;
    } // else if
    else {
        cout << "error" << endl ;
    } // else

    return node ;
} // CreateTree()

void Print_whitespace( int white_space ) {
    for ( int i = 0 ; i < white_space ; i++ )
        cout << "  " ;
} // Print_whitespace()

void Accurate_Print( string content ) {
    bool neg = false ;
    int value_int = 0 ;
    float value_float = 0 ;
    int backslash = 0 ;
    int bb = 0 ;
    string temp = "\0" ;

    if ( WhichType( content ) == FLOAT ) {
        if ( content[0] == '.' ) content = "0" + content ;

        printf( "%.3f", atof( content.c_str() ) ) ;
        cout << endl ;
    } // if
    else if ( WhichType( content ) == INT ) { // +123
        if ( content[0] == '+' ) content.erase( content.begin() ) ;

        cout << content << endl ;
    } // if
    else if ( WhichType( content ) == T ) {
        content = "#t" ;
        cout << content << endl ;
    } // else if
    else if ( WhichType( content ) == NIL || content == "()" ) {
        content = "nil" ;
        cout << content << endl ;
    } // else if
    else if ( WhichType( content ) == STRING ) {
        for ( int i = 0 ; i < content.size() ; i++ ) {
            if ( content[i] == '\\' ) backslash++ ;
            else if ( content[i] != '\\' && backslash > 0 ) {
                bb = backslash / 2 + 1 ;
                if ( backslash % 2 == 0 ) {
                    for ( int j = 1 ; j < bb ; j++ )
                        temp = temp + '\\' ;

                    temp = temp + content[i] ;
                } // if
                else {
                    for ( int j = 1 ; j < bb ; j++ )
                        temp = temp + '\\' ;

                    if ( content[i] == 'n' ) {
                        cout << temp ;
                        cout << "\n" ;
                        temp = "\0" ;
                    } // if
                    else if ( content[i] == '"' ) temp = temp + "\"" ;
                    else if ( content[i] == 't' ) temp = temp + "\t" ;
                    else if ( content[i] == '\\' ) temp = temp + "\\" ;
                    else {
                        temp = temp + "\\" ;
                        temp = temp + content[i] ;
                    } // else
                } // else

                backslash = 0 ;
            } // else if
            else {
                backslash = 0 ;
                temp = temp + content[i] ;
            } // else
        } // for

        content = temp ;
        cout << content << endl ;
    } // else if
    else if ( WhichType( content ) == QUOTE ) {
        content = "quote" ;
        cout << content << endl ;
    } // else if
    else {
        cout << content << endl ;
    } // else

} // Accurate_Print()

void NeedToExit( TreePtr* root ) {
    if ( root->m_left != NULL ) {
        if ( root->m_left->m_content == "exit" && IsNil( root->m_right->m_content ) ) {
            throw "Error for exit" ;
        } // if

        if ( root->m_left->m_content == "clean-environment" && IsNil( root->m_right->m_content ) ) {
            throw "clean-environment" ;
        } // if
    } // if
} // NeedToExit()

void PrettyPrint( TreePtr* node, int white_space ) {
    if ( node->m_left == NULL ) {
        Accurate_Print( node->m_content ) ;
        return ;
    } // if

    white_space++ ;
    cout << "( " ;
    if ( node->m_left != NULL ) {
        if ( node->m_left->m_content != "\0" ) Accurate_Print( node->m_left->m_content ) ;
        else PrettyPrint( node->m_left, white_space ) ; // left is internal node

    } // if

    if ( node->m_right != NULL ) {
        if ( node->m_right->m_content != "\0" && !IsNil( node->m_right->m_content ) ) {
            Print_whitespace( white_space ) ;
            cout << "." << endl ;
            Print_whitespace( white_space ) ;
            Accurate_Print( node->m_right->m_content ) ;
        } // if
        else {
            while ( node->m_right->m_content == "" ) {
                node = node->m_right ;
                if ( node->m_left->m_content != "\0" ) {
                    Print_whitespace( white_space ) ;
                    Accurate_Print( node->m_left->m_content ) ;
                } // if
                else {
                    Print_whitespace( white_space ) ;
                    PrettyPrint( node->m_left, white_space ) ;
                } // else

                if ( node->m_right->m_content != "\0" && !IsNil( node->m_right->m_content ) ) {
                    Print_whitespace( white_space ) ;
                    cout << "." << endl ;
                    Print_whitespace( white_space ) ;
                    Accurate_Print( node->m_right->m_content ) ;
                } // else if

            } // while

        } // else
    } // if

    Print_whitespace( white_space - 1 ) ;
    cout << ")" << endl ;
} // PrettyPrint()

bool IsSameToken( Token tok1, Token tok2 ) {
    if ( tok1.m_column == tok2.m_column && tok1.m_line == tok2.m_line && tok1.m_str == tok2.m_str )
        return true ;
    else return false ;
} // IsSameToken()

bool IsAtomNotString( string str ) {
    if ( WhichType( str ) == INT || WhichType( str ) == FLOAT ||
        WhichType( str ) == T || WhichType( str ) == NIL || str == "()" ) // 
        return true ;
    else return false ;
} // IsAtomNotString()

void SetCurrentLocation( vector<Token>& temp_cluster_token, int& cur_line,
    int& cur_column, int& left_token_size ) {
    Token temp_token ;
    if ( !temp_cluster_token.empty() ) {
        temp_token = temp_cluster_token.at( temp_cluster_token.size() - 1 ) ;
        cur_line = temp_token.m_line ;
        cur_column = temp_token.m_column ;
        if ( temp_token.m_str == "()" ) left_token_size = 1 ;
        else left_token_size = temp_token.m_str.size() ;
    } // if
} // SetCurrentLocation()

void DeleteSameLineContent( vector<Token>& cluster_token, vector<Token>& answer_cluster_token ) {
    int delete_line = cluster_token[0].m_line ;
    while ( cluster_token[0].m_line == delete_line )
        if ( cluster_token[0].m_str != "" ) {
            answer_cluster_token.push_back( cluster_token[0] ) ;
            cluster_token.erase( cluster_token.begin() ) ;
        } // if
        else delete_line++ ;
} // DeleteSameLineContent()

bool IsDistinquish( TreePtr* temp ) {
    for ( int i = 6 ; i < 16 ; i++ )
        if ( temp == u_cluster_Procedure_Symbol[i].m_root ) return true ;

    return false ;
} // IsDistinquish()

bool IsDefined_ProcedureSymbol( TreePtr* temp ) {
    for ( int i = 0 ; i < u_cluster_Defined_Symbol.size() ; i++ )
        if ( u_cluster_Defined_Symbol[i].m_content == temp->m_content ) return true ;

    for ( int i = 0 ; i < u_cluster_Procedure_Symbol.size() ; i++ )
        if ( u_cluster_Procedure_Symbol[i].m_content == temp->m_content ) return true ;

    return false ;
} // IsDefined_ProcedureSymbol()

bool IsProcedure( string str ) {
    for ( int i = 0 ; i < u_cluster_Procedure_Symbol.size() ; i++ )
        if ( u_cluster_Procedure_Symbol[i].m_content == str ) return true ;

    if ( WhichType( str ) == T || IsNil( str ) ) return true ;

    return false ;

} // IsProcedure()

bool IsList( TreePtr* temp ) {
    if ( temp->m_content != "" ) return true ;

    TreePtr* ansTemp = new TreePtr ;
    InitializeTree( ansTemp ) ;
    while ( temp->m_content == "" )
        temp = temp->m_right ;

    if ( IsNil( temp->m_content ) )  return true ;
    else return false ;

} // IsList()

bool IsAtomButNotSymbol( string str ) {
    if ( WhichType( str ) == STRING || WhichType( str ) == INT ||
        WhichType( str ) == FLOAT || WhichType( str ) == NIL ||
        WhichType( str ) == T || str == "()" )
        return true ;
    else return false ;
} // IsAtomButNotSymbol()

TreePtr* SymbolToData( TreePtr* temp, string symbolName ) {
    for ( int i = 0 ; i < u_cluster_Defined_Symbol.size() ; i++ )
        if ( u_cluster_Defined_Symbol[i].m_content == symbolName )
            return u_cluster_Defined_Symbol[i].m_root ;

    for ( int i = 0 ; i < u_cluster_Procedure_Symbol.size() ; i++ )
        if ( u_cluster_Procedure_Symbol[i].m_content == symbolName )
            return u_cluster_Procedure_Symbol[i].m_root ;

    return NULL ;
} // SymbolToData()

TreePtr* DoCons( TreePtr* temp ) {

    temp = temp->m_right ;
    temp->m_left = EvaluateTree( temp->m_left, 0 ) ;
    temp->m_right = EvaluateTree( temp->m_right->m_left, 0 ) ;

    return temp ;
} // DoCons()

TreePtr* DoList( TreePtr* temp ) {

    temp = temp->m_right ;
    TreePtr* temp_right = temp ;
    while ( !IsNil( temp_right->m_content ) ) {
        temp_right->m_left = EvaluateTree( temp_right->m_left, 0 ) ;
        temp_right = temp_right->m_right ;
    } // while

    return temp ;
} // DoList()

TreePtr* DoCar( TreePtr* temp ) {

    temp = EvaluateTree( temp->m_right->m_left, 0 ) ;
    return temp->m_left ;
} // DoCar()

TreePtr* DoCdr( TreePtr* temp ) {

    temp = EvaluateTree( temp->m_right->m_left, 0 ) ;
    return temp->m_right ;
} // DoCdr()

TreePtr* DoQuote( TreePtr* temp ) {

    temp = temp->m_right->m_left ;
    return temp ;
} // DoQuote()

bool SameDefinedSymbol( Special_Symbol sym ) {
    for ( int i = 0 ; i < u_cluster_Defined_Symbol.size() ; i++ )
        if ( u_cluster_Defined_Symbol[i].m_content == sym.m_content ) {
            u_cluster_Defined_Symbol[i].m_root = sym.m_root ;
            return true ;
        } // if

    return false ;
} // SameDefinedSymbol()

void DoDefine( TreePtr* temp ) {

    Special_Symbol sym ;

    sym.m_content = temp->m_right->m_left->m_content ;
    sym.m_root = EvaluateTree( temp->m_right->m_right->m_left, 0 ) ;
    if ( !SameDefinedSymbol( sym ) ) u_cluster_Defined_Symbol.push_back( sym ) ;
    cout << "> " << sym.m_content << " defined" << endl << endl ;
    throw "got defined" ;

} // DoDefine()

TreePtr* AnalyAtom( TreePtr* temp ) {
    temp->m_right->m_left = EvaluateTree( temp->m_right->m_left, 0 ) ;
    int x = WhichType( temp->m_right->m_left->m_content ) ;
    TreePtr* ansTemp = new TreePtr ;
    InitializeTree( ansTemp ) ;
    if ( x == SYMBOL || x == INT || x == FLOAT || x == STRING || x == NIL || x == T
        || temp->m_right->m_left->m_content == "()" )
        ansTemp->m_content = "#t" ;
    else ansTemp->m_content = "nil" ;

    return ansTemp ;
} // AnalyAtom()

TreePtr* AnalyPair( TreePtr* temp ) {
    temp->m_right->m_left = EvaluateTree( temp->m_right->m_left, 0 ) ;
    TreePtr* ansTemp = new TreePtr ;
    InitializeTree( ansTemp ) ;
    if ( temp->m_right->m_left->m_content == "" ) ansTemp->m_content = "#t" ;
    else ansTemp->m_content = "nil" ;

    return ansTemp ;
} // AnalyPair()

TreePtr* AnalyList( TreePtr* temp ) {
    temp->m_right->m_left = EvaluateTree( temp->m_right->m_left, 0 ) ;
    TreePtr* ansTemp = new TreePtr ;
    InitializeTree( ansTemp ) ;
    if ( temp->m_right->m_left->m_content == "" ) {
        temp = temp->m_right->m_left ;
        while ( temp->m_right->m_content == "" )
            temp = temp->m_right ;

        if ( IsNil( temp->m_right->m_content ) )  ansTemp->m_content = "#t" ;
        else ansTemp->m_content = "nil" ;
    } // if
    else ansTemp->m_content = "nil" ;

    return ansTemp ;
} // AnalyList()

TreePtr* AnalyNull( TreePtr* temp ) {
    temp->m_right->m_left = EvaluateTree( temp->m_right->m_left, 0 ) ;
    TreePtr* ansTemp = new TreePtr ;
    InitializeTree( ansTemp ) ;
    if ( IsNil( temp->m_right->m_left->m_content ) )
        ansTemp->m_content = "#t" ;
    else ansTemp->m_content = "nil" ;

    return ansTemp ;
} // AnalyNull()

TreePtr* AnalyInteger( TreePtr* temp ) {
    temp->m_right->m_left = EvaluateTree( temp->m_right->m_left, 0 ) ;
    TreePtr* ansTemp = new TreePtr ;
    InitializeTree( ansTemp ) ;
    if ( WhichType( temp->m_right->m_left->m_content ) == INT )
        ansTemp->m_content = "#t" ;
    else ansTemp->m_content = "nil" ;

    return ansTemp ;
} // AnalyInteger()

TreePtr* AnalyReal( TreePtr* temp ) {
    temp->m_right->m_left = EvaluateTree( temp->m_right->m_left, 0 ) ;
    TreePtr* ansTemp = new TreePtr ;
    InitializeTree( ansTemp ) ;
    if ( WhichType( temp->m_right->m_left->m_content ) == INT ||
        WhichType( temp->m_right->m_left->m_content ) == FLOAT )
        ansTemp->m_content = "#t" ;
    else ansTemp->m_content = "nil" ;

    return ansTemp ;
} // AnalyReal()

TreePtr* AnalyNumber( TreePtr* temp ) {
    temp->m_right->m_left = EvaluateTree( temp->m_right->m_left, 0 ) ;
    TreePtr* ansTemp = new TreePtr ;
    InitializeTree( ansTemp ) ;
    if ( WhichType( temp->m_right->m_left->m_content ) == INT ||
        WhichType( temp->m_right->m_left->m_content ) == FLOAT )
        ansTemp->m_content = "#t" ;
    else ansTemp->m_content = "nil" ;

    return ansTemp ;
} // AnalyNumber()

TreePtr* AnalyString( TreePtr* temp ) {
    temp->m_right->m_left = EvaluateTree( temp->m_right->m_left, 0 ) ;
    TreePtr* ansTemp = new TreePtr ;
    InitializeTree( ansTemp ) ;
    if ( WhichType( temp->m_right->m_left->m_content ) == STRING )
        ansTemp->m_content = "#t" ;
    else ansTemp->m_content = "nil" ;

    return ansTemp ;
} // AnalyString()

TreePtr* AnalyBoolean( TreePtr* temp ) {
    temp->m_right->m_left = EvaluateTree( temp->m_right->m_left, 0 ) ;
    TreePtr* ansTemp = new TreePtr ;
    InitializeTree( ansTemp ) ;
    if ( WhichType( temp->m_right->m_left->m_content ) == T ||
        IsNil( temp->m_right->m_left->m_content ) )
        ansTemp->m_content = "#t" ;
    else ansTemp->m_content = "nil" ;

    return ansTemp ;
} // AnalyBoolean()

TreePtr* AnalySymbol( TreePtr* temp ) {
    temp->m_right->m_left = EvaluateTree( temp->m_right->m_left, 2 ) ;
    TreePtr* ansTemp = new TreePtr ;
    InitializeTree( ansTemp ) ;
    if ( WhichType( temp->m_right->m_left->m_content ) == SYMBOL )
        ansTemp->m_content = "#t" ;
    else ansTemp->m_content = "nil" ;

    return ansTemp ;
} // AnalySymbol()

TreePtr* DoOp( TreePtr* temp, string op ) {

    TreePtr* ansTemp = new TreePtr ;
    InitializeTree( ansTemp ) ;
    float answer = 0 ;
    TreePtr* answer_tree_str = new TreePtr ;
    InitializeTree( answer_tree_str ) ;
    bool isFloat = false ;
    bool firstisfloat = false ;
    temp = temp->m_right ;

    answer_tree_str = EvaluateTree( temp->m_left, 0 ) ;
    answer = answer + atof( answer_tree_str->m_content.c_str() ) ;
    if ( WhichType( answer_tree_str->m_content ) == FLOAT ) {
        isFloat = true ;
        firstisfloat = true ;
    } // if 

    temp = temp->m_right ;
    while ( !IsNil( temp->m_content ) ) {
        answer_tree_str = EvaluateTree( temp->m_left, 0 ) ;
        if ( ( WhichType( answer_tree_str->m_content ) == INT ||
            WhichType( answer_tree_str->m_content ) == FLOAT ) && op == "/" ) {
            if ( atof( answer_tree_str->m_content.c_str() ) == 0 ) {
                cout << "> ERROR (division by zero) : /" << endl << endl ;
                throw "ERROR for p2" ;
            } // if

            else {
                if ( WhichType( answer_tree_str->m_content ) == INT && !firstisfloat ) {
                    int answer_int = answer  ;
                    int temp_int = atoi( answer_tree_str->m_content.c_str() ) ;
                    answer_int = answer_int / temp_int ;
                    answer = answer_int ;
                } // if
                else {
                    isFloat = true ;
                    answer = answer / atof( answer_tree_str->m_content.c_str() ) ;
                } // else

            } // else
        } // if 
        else if ( WhichType( answer_tree_str->m_content ) == INT && op == "*" )
            answer = answer * atof( answer_tree_str->m_content.c_str() ) ;
        else if ( WhichType( answer_tree_str->m_content ) == FLOAT && op == "*" ) {
            answer = answer * atof( answer_tree_str->m_content.c_str() ) ;
            isFloat = true ;
        } // else if
        else if ( WhichType( answer_tree_str->m_content ) == INT && op == "+" )
            answer = answer + atof( answer_tree_str->m_content.c_str() ) ;
        else if ( WhichType( answer_tree_str->m_content ) == FLOAT && op == "+" ) {
            answer = answer + atof( answer_tree_str->m_content.c_str() ) ;
            isFloat = true ;
        } // else if
        else if ( WhichType( answer_tree_str->m_content ) == INT && op == "-" )
            answer = answer - atof( answer_tree_str->m_content.c_str() ) ;
        else if ( WhichType( answer_tree_str->m_content ) == FLOAT && op == "-" ) {
            answer = answer - atof( answer_tree_str->m_content.c_str() ) ;
            isFloat = true ;
        } // else if

        temp = temp->m_right ;
        firstisfloat = false ;
    } // while


    stringstream sstream ;
    sstream << answer ;
    if ( isFloat == true && WhichType( sstream.str() ) == INT )
        ansTemp->m_content = sstream.str() + ".000" ;
    else
        ansTemp->m_content = sstream.str() ;

    return ansTemp ;
} // DoOp()

TreePtr* DoNot( TreePtr* temp ) {

    TreePtr* ansTemp = new TreePtr ;
    InitializeTree( ansTemp ) ;
    string answer = "\0" ;
    temp = temp->m_right ;
    answer = EvaluateTree( temp->m_left, 0 )->m_content ;


    if ( IsNil( answer ) ) ansTemp->m_content = "#t" ;
    else ansTemp->m_content = "nil" ;

    return ansTemp ;
} // DoNot()

TreePtr* DoAnd( TreePtr* temp ) {

    TreePtr* ansTemp = new TreePtr ;
    InitializeTree( ansTemp ) ;
    string answer = "\0" ;
    bool isBreak = false ;

    temp = temp->m_right ;
    while ( !IsNil( temp->m_content ) && !isBreak ) {
        answer = EvaluateTree( temp->m_left, 0 )->m_content ;
        if ( IsNil( answer ) )  isBreak = true ;
        temp = temp->m_right ;
    } // while

    ansTemp->m_content = answer ;

    return ansTemp ;
} // DoAnd()

TreePtr* DoOr( TreePtr* temp ) {

    TreePtr* ansTemp = new TreePtr ;
    InitializeTree( ansTemp ) ;
    string answer = "\0" ;
    bool isBreak = false ;

    temp = temp->m_right ;
    while ( !IsNil( temp->m_content ) && !isBreak ) {
        answer = EvaluateTree( temp->m_left, 0 )->m_content ;
        if ( !IsNil( answer ) )  isBreak = true ;
        temp = temp->m_right ;
    } // while

    ansTemp->m_content = answer ;

    return ansTemp ;
} // DoOr()

TreePtr* DoCompare( TreePtr* temp, string op ) {

    TreePtr* ansTemp = new TreePtr ;
    InitializeTree( ansTemp ) ;

    temp = temp->m_right ;
    TreePtr* answer_tree_str = new TreePtr ;
    InitializeTree( answer_tree_str ) ;
    answer_tree_str = EvaluateTree( temp->m_left, 0 ) ;
    string str1 = answer_tree_str->m_content ;
    string str2 = "\0" ;
    float num_str1 = 0 ;
    float num_str2 = 0 ;
    bool isBreak = false ;
    bool ans = true ;
    if ( WhichType( str1 ) != INT && WhichType( str1 ) != FLOAT ) {
        cout << "> ERROR (" << op << " with incorrect argument type) : " ;
        PrettyPrint( answer_tree_str, 0 ) ;
        cout << endl ;
        throw "ERROR for p2" ;
    } // if 

    num_str1 = atof( str1.c_str() ) ;
    temp = temp->m_right ;
    while ( !IsNil( temp->m_content ) ) {
        answer_tree_str = EvaluateTree( temp->m_left, 0 ) ;
        str2 = answer_tree_str->m_content ;
        if ( !isBreak ) {
            num_str2 = atof( str2.c_str() ) ;
            if ( op == ">=" ) {
                if ( num_str1 >= num_str2 ) ans = true ;
                else {
                    ans = false ;
                    isBreak = true ;
                } // else
            } // if
            else if ( op == ">" ) {
                if ( num_str1 > num_str2 ) ans = true ;
                else {
                    ans = false ;
                    isBreak = true ;
                } // else
            } // else if
            else if ( op == "=" ) {
                if ( num_str1 == num_str2 ) ans = true ;
                else {
                    ans = false ;
                    isBreak = true ;
                } // else
            } // else if
            else if ( op == "<" ) {
                if ( num_str1 < num_str2 ) ans = true ;
                else {
                    ans = false ;
                    isBreak = true ;
                } // else
            } // else if
            else if ( op == "<=" ) {
                if ( num_str1 <= num_str2 ) ans = true ;
                else {
                    ans = false ;
                    isBreak = true ;
                } // else
            } // else if
            else ;


            num_str1 = num_str2 ;
        } // if
        else ;

        temp = temp->m_right ;
    } // while

    if ( ans == true ) ansTemp->m_content = "#t" ;
    else ansTemp->m_content = "nil" ;

    return ansTemp ;
} // DoCompare()

TreePtr* DoStrCompare( TreePtr* temp, string op ) {

    TreePtr* ansTemp = new TreePtr ;
    InitializeTree( ansTemp ) ;

    temp = temp->m_right ;
    TreePtr* answer_tree_str = new TreePtr ;
    InitializeTree( answer_tree_str ) ;
    answer_tree_str = EvaluateTree( temp->m_left, 0 ) ;

    string str1 = answer_tree_str->m_content ;
    string str2 = "\0" ;
    bool isBreak = false ;
    bool ans = true ;

    temp = temp->m_right ;
    while ( !IsNil( temp->m_content ) ) {
        answer_tree_str = EvaluateTree( temp->m_left, 0 ) ;
        str2 = answer_tree_str->m_content ;
        if ( !isBreak ) {
            if ( op == "string>?" ) {
                if ( str1 > str2 ) ans = true ;
                else {
                    ans = false ;
                    isBreak = true ;
                } // else
            } // if
            else if ( op == "string=?" ) {
                if ( str1 == str2 ) ans = true ;
                else {
                    ans = false ;
                    isBreak = true ;
                } // else
            } // else if
            else if ( op == "string<?" ) {
                if ( str1 < str2 ) ans = true ;
                else {
                    ans = false ;
                    isBreak = true ;
                } // else
            } // else if

            str1 = str2 ;
        } // if
        else ;

        temp = temp->m_right ;
    } // while

    if ( ans == true ) ansTemp->m_content = "#t" ;
    else ansTemp->m_content = "nil" ;

    return ansTemp ;
} // DoStrCompare()

TreePtr* DoStrApp( TreePtr* temp ) {

    TreePtr* ansTemp = new TreePtr ;
    InitializeTree( ansTemp ) ;
    string answer_str = "\0" ;
    string temp_str = "\0" ;
    temp = temp->m_right ;
    TreePtr* answer_tree_str = new TreePtr ;
    InitializeTree( answer_tree_str ) ;
    answer_tree_str = EvaluateTree( temp->m_left, 0 ) ;
    temp_str = answer_tree_str->m_content ;

    temp_str.erase( temp_str.begin() ) ;
    temp_str.erase( temp_str.begin() + temp_str.size() - 1 ) ;
    answer_str = answer_str + temp_str ;


    temp = temp->m_right ;
    while ( !IsNil( temp->m_content ) ) {
        answer_tree_str = EvaluateTree( temp->m_left, 0 ) ;
        temp_str = answer_tree_str->m_content ;
        temp_str.erase( temp_str.begin() + temp_str.size() - 1 ) ;
        temp_str.erase( temp_str.begin() ) ;
        answer_str = answer_str + temp_str ;

        temp = temp->m_right ;
    } // while

    answer_str = "\"" + answer_str + "\"" ;
    ansTemp->m_content = answer_str ;

    return ansTemp ;
} // DoStrApp()

TreePtr* DoEqv( TreePtr* temp ) {


    TreePtr* ansTemp = new TreePtr ;
    InitializeTree( ansTemp ) ;
    TreePtr* temp1 = EvaluateTree( temp->m_right->m_left, 0 )  ;
    TreePtr* temp2 = EvaluateTree( temp->m_right->m_right->m_left, 0 ) ;
    if ( temp1 == temp2 ) ansTemp->m_content = "#t" ;
    else if ( WhichType( temp1->m_content ) == INT && WhichType( temp2->m_content ) == INT ) {
        int t1 = atoi( temp1->m_content.c_str() ) ;
        int t2 = atoi( temp2->m_content.c_str() ) ;
        if ( t1 == t2 ) ansTemp->m_content = "#t" ;
        else ansTemp->m_content = "nil" ;
    } // else if
    else if ( WhichType( temp1->m_content ) == FLOAT && WhichType( temp2->m_content ) == FLOAT ) {
        float t1 = atof( temp1->m_content.c_str() ) ;
        float t2 = atof( temp2->m_content.c_str() ) ;
        if ( t1 == t2 ) ansTemp->m_content = "#t" ;
        else ansTemp->m_content = "nil" ;
    } // else if
    else if ( IsAtomNotString( temp1->m_content ) && IsAtomNotString( temp2->m_content ) &&
        temp1->m_content == temp2->m_content )
        ansTemp->m_content = "#t" ;
    else ansTemp->m_content = "nil" ;

    return ansTemp ;

} // DoEqv()

bool SameTree( TreePtr* temp1, TreePtr* temp2 ) {
    if ( temp1 == NULL && temp2 == NULL ) return true ;
    else if ( temp1 == NULL && temp2 != NULL ) return false ;
    else if ( temp1 != NULL && temp2 == NULL ) return false ;
    else if ( temp1->m_content != "" && temp2->m_content != "" &&
        WhichType( temp1->m_content ) == INT && WhichType( temp2->m_content ) == INT ) {
        int t1 = atoi( temp1->m_content.c_str() ) ;
        int t2 = atoi( temp2->m_content.c_str() ) ;
        if ( t1 == t2 ) return true ;
        else return false ;
    } // else if
    else if ( temp1->m_content != "" && temp2->m_content != "" &&
        WhichType( temp1->m_content ) == FLOAT && WhichType( temp2->m_content ) == FLOAT ) {
        float t1 = atof( temp1->m_content.c_str() ) ;
        float t2 = atof( temp2->m_content.c_str() ) ;
        if ( t1 == t2 ) return true ;
        else return false ;
    } // else if
    else if ( temp1->m_content != "" && temp2->m_content != ""
        && temp1->m_content == temp2->m_content )
        return true ;
    else if ( temp1->m_content != temp2->m_content ) return false ;
    else if ( SameTree( temp1->m_left, temp2->m_left ) && SameTree( temp1->m_right, temp2->m_right ) )
        return true ;
    else return false ;
} // SameTree()

TreePtr* DoEqual( TreePtr* temp ) {

    TreePtr* ansTemp = new TreePtr ;
    InitializeTree( ansTemp ) ;
    TreePtr* temp1 = EvaluateTree( temp->m_right->m_left, 0 )  ;
    TreePtr* temp2 = EvaluateTree( temp->m_right->m_right->m_left, 0 ) ;

    if ( SameTree( temp1, temp2 ) )
        ansTemp->m_content = "#t" ;
    else ansTemp->m_content = "nil" ;

    return ansTemp ;

} // DoEqual()

TreePtr* Distinquish( TreePtr* temp, TreePtr* funct ) {

    if ( funct == u_cluster_Procedure_Symbol[6].m_root ) temp = AnalyAtom( temp )  ;
    else if ( funct == u_cluster_Procedure_Symbol[7].m_root ) temp = AnalyPair( temp )  ;
    else if ( funct == u_cluster_Procedure_Symbol[8].m_root ) temp = AnalyList( temp )  ;
    else if ( funct == u_cluster_Procedure_Symbol[9].m_root ) temp = AnalyNull( temp )  ;
    else if ( funct == u_cluster_Procedure_Symbol[10].m_root ) temp = AnalyInteger( temp )  ;
    else if ( funct == u_cluster_Procedure_Symbol[11].m_root ) temp = AnalyReal( temp )  ;
    else if ( funct == u_cluster_Procedure_Symbol[12].m_root ) temp = AnalyNumber( temp )  ;
    else if ( funct == u_cluster_Procedure_Symbol[13].m_root ) temp = AnalyString( temp )  ;
    else if ( funct == u_cluster_Procedure_Symbol[14].m_root ) temp = AnalyBoolean( temp )  ;
    else if ( funct == u_cluster_Procedure_Symbol[15].m_root ) temp = AnalySymbol( temp ) ;
    else cout << "ERROR : fuckng error" << endl ;

    return temp ;
} // Distinquish()

TreePtr* DoIf( TreePtr* temp ) {
    TreePtr* root = temp ;

    temp = temp->m_right ;
    TreePtr* boolean_tree = EvaluateTree( temp->m_left, 0 ) ;
    TreePtr* answer = NULL ;
    temp = temp->m_right ;

    if ( !IsNil( boolean_tree->m_content ) )
        answer = EvaluateTree( temp->m_left, 0 ) ;
    else {
        answer = EvaluateTree( temp->m_right->m_left, 0 ) ;
    } // else

    return answer ;

} // DoIf()

TreePtr* DoCond( TreePtr* temp ) {
    TreePtr* root = temp ;

    int level = 0 ;
    TreePtr* t_left = NULL ;
    for ( TreePtr* t = temp->m_right ; !IsNil( t->m_content ) ; t = t->m_right ) {
        t_left = t->m_left ;
        level++ ;
    } // for

    TreePtr* answer = new TreePtr ;
    InitializeTree( answer ) ;
    bool gotAns = false ;

    temp = temp->m_right ;
    TreePtr* templeft = temp->m_left ;
    for ( int i = 0 ; i < level - 1 && !gotAns ; i++ ) {

        if ( !IsNil( EvaluateTree( templeft->m_left, 0 )->m_content ) ) {
            templeft = templeft->m_right ;
            while ( !IsNil( templeft->m_content ) ) {
                answer = EvaluateTree( templeft->m_left, 0 ) ;
                templeft = templeft->m_right ;
            } // while

            gotAns = true ;
        } // if

        temp = temp->m_right ;
        templeft = temp->m_left ;
    } // for

    if ( gotAns == false ) {
        if ( templeft->m_left->m_content == "else" ||
            !IsNil( EvaluateTree( templeft->m_left, 0 )->m_content ) ) {
            templeft = templeft->m_right ;
            while ( !IsNil( templeft->m_content ) ) {
                answer = EvaluateTree( templeft->m_left, 0 ) ;
                templeft = templeft->m_right ;
            } // while

            gotAns = true ;
        } // if
    } // if

    if ( gotAns == false ) {
        cout << "> ERROR (no return value) : " ;
        PrettyPrint( root, 0 ) ;
        cout << endl ;
        throw "ERROR for p2" ;
    } // if

    return answer ;
} // DoCond()

TreePtr* DoBegin( TreePtr* temp ) {

    TreePtr* answer = new TreePtr ;
    InitializeTree( answer ) ;
    temp = temp->m_right ;
    while ( !IsNil( temp->m_content ) ) {
        answer = EvaluateTree( temp->m_left, 0 ) ;
        temp = temp->m_right ;
    } // while

    return answer ;
} // DoBegin()

void DoExit( TreePtr* temp ) {

    if ( IsNil( temp->m_right->m_content ) ) {
        throw "Error for exit" ;
    } // if

} // DoExit()

void DoCleanEn( TreePtr* temp ) {

    if ( IsNil( temp->m_right->m_content ) ) {
        throw "clean-environment" ;
    } // if

} // DoCleanEn()

void NotLevel( string str ) {
    cout << "> ERROR (level of " << str << ")" << endl << endl ;
    throw "ERROR for p2" ;
} // NotLevel()

bool IsProcedureRoot( TreePtr* temp ) {
    for ( int i = 0 ; i < u_cluster_Procedure_Symbol.size() ; i++ )
        if ( temp == u_cluster_Procedure_Symbol[i].m_root ) return true ;

    return false ;
} // IsProcedureRoot()

void Translate_token( TreePtr* temp ) {
    if ( IsNil( temp->m_content ) ) temp->m_content = "nil" ;
    else if ( WhichType( temp->m_content ) == T ) temp->m_content = "#t" ;
} // Translate_token()

TreePtr* EvaluateTree( TreePtr* root, int FirstDefine ) {
    if ( root->m_left == NULL || root->m_right == NULL ) {
        Translate_token( root ) ;
        if ( WhichType( root->m_content ) == SYMBOL ) {
            if ( IsDefined_ProcedureSymbol( root ) ) {
                root = SymbolToData( root, root->m_content ) ;
            } // if
        } // if

        return root ;
    } // if

    TreePtr* temp = root ;
    TreePtr* funct = EvaluateTree( temp->m_left, 0 ) ;
    // string funct = EvaluateTree( temp->m_left, 0 )->m_content ;
    // cout << " funct :  " << funct->m_content << endl ;


    if ( funct == u_cluster_Procedure_Symbol[0].m_root ) temp = DoCons( temp ) ;
    else if ( funct == u_cluster_Procedure_Symbol[1].m_root ) temp = DoList( temp ) ;
    else if ( funct == u_cluster_Procedure_Symbol[2].m_root ) temp = DoQuote( temp ) ;
    else if ( funct == u_cluster_Procedure_Symbol[3].m_root && FirstDefine == 1 ) DoDefine( temp ) ;
    else if ( funct == u_cluster_Procedure_Symbol[3].m_root && FirstDefine == 0 ) NotLevel( "DEFINE" );
    else if ( funct == u_cluster_Procedure_Symbol[4].m_root ) temp = DoCar( temp ) ;
    else if ( funct == u_cluster_Procedure_Symbol[5].m_root ) temp = DoCdr( temp ) ;
    else if ( IsDistinquish( funct ) ) temp = Distinquish( temp, funct ) ;
    else if ( funct == u_cluster_Procedure_Symbol[16].m_root ) temp = DoOp( temp, "+" ) ;
    else if ( funct == u_cluster_Procedure_Symbol[17].m_root ) temp = DoOp( temp, "-" ) ;
    else if ( funct == u_cluster_Procedure_Symbol[18].m_root ) temp = DoOp( temp, "*" ) ;
    else if ( funct == u_cluster_Procedure_Symbol[19].m_root ) temp = DoOp( temp, "/" ) ;
    else if ( funct == u_cluster_Procedure_Symbol[20].m_root ) temp = DoNot( temp ) ;
    else if ( funct == u_cluster_Procedure_Symbol[21].m_root ) temp = DoAnd( temp ) ;
    else if ( funct == u_cluster_Procedure_Symbol[22].m_root ) temp = DoOr( temp ) ;
    else if ( funct == u_cluster_Procedure_Symbol[23].m_root ) temp = DoCompare( temp, ">" ) ;
    else if ( funct == u_cluster_Procedure_Symbol[24].m_root ) temp = DoCompare( temp, ">=" ) ;
    else if ( funct == u_cluster_Procedure_Symbol[25].m_root ) temp = DoCompare( temp, "<" ) ;
    else if ( funct == u_cluster_Procedure_Symbol[26].m_root ) temp = DoCompare( temp, "<=" ) ;
    else if ( funct == u_cluster_Procedure_Symbol[27].m_root ) temp = DoCompare( temp, "=" ) ;
    else if ( funct == u_cluster_Procedure_Symbol[28].m_root ) temp = DoStrApp( temp ) ;
    else if ( funct == u_cluster_Procedure_Symbol[29].m_root ) temp = DoStrCompare( temp, "string>?" ) ;
    else if ( funct == u_cluster_Procedure_Symbol[30].m_root ) temp = DoStrCompare( temp, "string<?" ) ;
    else if ( funct == u_cluster_Procedure_Symbol[31].m_root ) temp = DoStrCompare( temp, "string=?" ) ;
    else if ( funct == u_cluster_Procedure_Symbol[32].m_root ) temp = DoEqv( temp ) ;
    else if ( funct == u_cluster_Procedure_Symbol[33].m_root ) temp = DoEqual( temp ) ;
    else if ( funct == u_cluster_Procedure_Symbol[34].m_root ) temp = DoBegin( temp ) ;
    else if ( funct == u_cluster_Procedure_Symbol[35].m_root ) temp = DoIf( temp ) ;
    else if ( funct == u_cluster_Procedure_Symbol[36].m_root ) temp = DoCond( temp ) ;
    else if ( funct == u_cluster_Procedure_Symbol[37].m_root && FirstDefine == 1 ) DoExit( temp ) ;
    // else if ( funct == u_cluster_Procedure_Symbol[37].m_root && FirstDefine == 0 ) NotLevel( "EXIT" ) ;
    else if ( funct == u_cluster_Procedure_Symbol[38].m_root && FirstDefine == 1 ) DoCleanEn( temp ) ;
    // else if ( funct == u_cluster_Procedure_Symbol[38].m_root && FirstDefine == 0 )
    //   NotLevel( "CLEAN-ENVIRONMENT" ) ;
    // else if ( IsDefined_ProcedureSymbol( funct ) ) temp = SymbolToData( funct, funct->m_content ) ;
    else {
        cout << "> ERROR (attempt to apply non-function) : " ;
        PrettyPrint( funct, 0 ) ;
        cout << endl ;
        throw "ERROR for p2" ;
    } // else 

    return temp ;
} // EvaluateTree()

void SetTree( string funct ) {
    Special_Symbol pri ;
    pri.m_content = funct ;
    pri.m_root = new TreePtr ;
    InitializeTree( pri.m_root ) ;
    pri.m_root->m_content = "#<procedure " + funct + ">" ;
    u_cluster_Procedure_Symbol.push_back( pri ) ;
} // SetTree()

void SetPrimitive() {
    SetTree( "cons" ) ; // 0
    SetTree( "list" ) ; // 1
    SetTree( "quote" ) ; // 2
    SetTree( "define" ) ; // 3
    SetTree( "car" ) ; // 4
    SetTree( "cdr" ) ; // 5
    SetTree( "atom?" ) ; // 6
    SetTree( "pair?" ) ; // 7
    SetTree( "list?" ) ; // 8
    SetTree( "null?" ) ; // 9
    SetTree( "integer?" ) ; // 10
    SetTree( "real?" ) ; // 11
    SetTree( "number?" ) ; // 12
    SetTree( "string?" ) ; // 13
    SetTree( "boolean?" ) ; // 14
    SetTree( "symbol?" ) ; // 15
    SetTree( "+" ) ; // 16
    SetTree( "-" ) ; // 17
    SetTree( "*" ) ; // 18
    SetTree( "/" ) ; // 19
    SetTree( "not" ) ; // 20
    SetTree( "and" ) ; // 21
    SetTree( "or" ) ; // 22
    SetTree( ">" ) ; // 23
    SetTree( ">=" ) ; // 24
    SetTree( "<" ) ; // 25
    SetTree( "<=" ) ; // 26
    SetTree( "=" ) ; // 27
    SetTree( "string-append" ) ; // 28
    SetTree( "string>?" ) ; // 29
    SetTree( "string<?" ) ; // 30
    SetTree( "string=?" ) ; // 31
    SetTree( "eqv?" ) ; // 32
    SetTree( "equal?" ) ; // 33
    SetTree( "begin" ) ; // 34
    SetTree( "if" ) ; // 35
    SetTree( "cond" ) ; // 36
    SetTree( "exit" ) ; // 37
    SetTree( "clean-environment" ) ; // 38

} // SetPrimitive()

int main()
{
    cout << "Welcome to OurScheme!\n\n" ;
    vector<Token> cluster_token ;
    vector<Token> temp_cluster_token ;
    vector<Token> answer_cluster_token ;
    TreePtr* root = NULL ;
    Token temp_token ;

    bool isBreak = false ;

    int cur_line = 0 ;
    int cur_column = 0 ;
    int left_token_size = 0 ;
    int cout_line = 0 ;
    int cout_column = 0 ;
    cin >> u_testNum ;
    if ( u_testNum == 200 ) cout << "fuck" ;
    else {
        cin.get() ;
        SetPrimitive() ;
        Collect_token( cluster_token ) ;
        while ( isBreak == false && !( cluster_token.empty() ) ) {
            try {
                u_first_exp_line = cluster_token[0].m_line ;
                u_first_exp_column = cluster_token[0].m_column ;
                Read_SExp( cluster_token, answer_cluster_token ) ;
                temp_cluster_token = answer_cluster_token ;
                root = NULL ;
                root = CreateTree( answer_cluster_token, root ) ;

                SetCurrentLocation( temp_cluster_token, cur_line, cur_column, left_token_size ) ;
                temp_cluster_token.clear() ;

                root = EvaluateTree( root, 1 ) ;
                cout << "> " ;
                PrettyPrint( root, 0 ) ;
                cout << endl ;
            } // try
            catch ( const char* error_message ) {

                if ( strcmp( "Error for no more input", error_message ) == 0 ) {
                    cout << "> ERROR (no more input) : END-OF-FILE encountered" << endl ;
                    isBreak = true ;
                } // else if 
                else if ( strcmp( "Error for syntax", error_message ) == 0 ) {

                    if ( cur_line == u_first_exp_line ) cout_line = cluster_token[0].m_line - cur_line + 1 ;
                    else cout_line = cluster_token[0].m_line - cur_line ;

                    if ( cluster_token[0].m_line == cur_line )
                        cout_column = cluster_token[0].m_column - cur_column - left_token_size + 1 ;
                    else
                        cout_column = cluster_token[0].m_column ;

                    cout << "> ERROR (unexpected token) : atom or '(' expected when token at Line " ;
                    cout << cout_line << " Column " << cout_column << " is >>" ;
                    cout << cluster_token[0].m_str << "<<" << endl << endl ;
                    DeleteSameLineContent( cluster_token, answer_cluster_token ) ;
                    SetCurrentLocation( answer_cluster_token, cur_line, cur_column, left_token_size ) ;
                    answer_cluster_token.clear() ;
                } // else if
                else if ( strcmp( "Error for syntax fo right paren", error_message ) == 0 ) {
                    if ( !answer_cluster_token.empty() ) {
                        if ( cluster_token[0].m_line == cur_line ) {
                            cout_line = cluster_token[0].m_line - cur_line + 1 ;
                            cout_column = cluster_token[0].m_column - cur_column - left_token_size + 1 ;
                        } // if
                        else {
                            cout_line = cluster_token[0].m_line - cur_line ;
                            cout_column = cluster_token[0].m_column ;
                        } // else
                    } // if

                    cout << "> ERROR (unexpected token) : ')' expected when token at Line " ;
                    cout << cout_line << " Column " << cout_column << " is >>" ;
                    cout << cluster_token[0].m_str << "<<" << endl << endl ;
                    DeleteSameLineContent( cluster_token, answer_cluster_token ) ;
                    SetCurrentLocation( answer_cluster_token, cur_line, cur_column, left_token_size ) ;
                    answer_cluster_token.clear() ;
                } // else if 
                else if ( strcmp( "error for close string", error_message ) == 0 ) {
                    if ( cluster_token[0].m_line == cur_line ) {
                        cout_line = 1 ;
                        cout_column = cluster_token[0].m_column - cur_column - left_token_size + 1 + 1 ;
                    } // if
                    else {
                        cout_line = cluster_token[0].m_line - cur_line ;
                        cout_column = cluster_token[0].m_column + 1 ;
                    } // else

                    cout << "> ERROR (no closing quote) : END-OF-LINE encountered at Line " << cout_line ;
                    cout << " Column " << cout_column << endl << endl ;
                    cluster_token.erase( cluster_token.begin() ) ;
                    SetCurrentLocation( answer_cluster_token, cur_line, cur_column, left_token_size ) ;
                    answer_cluster_token.clear() ;
                } // else if
                else if ( strcmp( "Error for exit", error_message ) == 0 ) {
                    cout << "> " << endl ;
                    isBreak = true ;
                } // else if

                else if ( strcmp( "got defined", error_message ) == 0 ) ;
                else if ( strcmp( "incorrect number", error_message ) == 0 ) ;
                else if ( strcmp( "non-list", error_message ) == 0 ) ;
                else if ( strcmp( "Unbounded Symbol", error_message ) == 0 ) ;
                else if ( strcmp( "non-function", error_message ) == 0 ) ;
                else if ( strcmp( "incorrect argument type", error_message ) == 0 ) ;
                else if ( strcmp( "ERROR for p2", error_message ) == 0 ) ;
                else if ( strcmp( "clean-environment", error_message ) == 0 ) {
                    cout << "> environment cleaned" << endl << endl ;
                    u_cluster_Defined_Symbol.clear() ;
                } // else if 
                else cout << "fuck" ;
            } // catch
        } // while

        cout << "Thanks for using OurScheme!" ;
    } // else


} // main()

