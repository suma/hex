
#include <string>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <QApplication>
#include <QtDebug>
#include "../control/document.h"

using namespace std;

class Test
{
public:
  const char *org_str;
  //StringOriginal org;
  Document *doc;
  string test;

private:
  int count;

public:
  Test(const char *s)
    : org_str( s )
    , count(0)
    //, org( str )
    //, piece( new PieceTable( org ) )
  {
    cout << "PieceTable<char> TestUnit\n\n";
  }


  void TestUnit()
  {
    cout << "## Initialize" << endl;
    Init( true );
    Dump();

    int length = doc->length();

    // Insert
    cout << "## Insert ";
    for ( int i = 0; i <= length; i++ ) {
      Init();
      string old = test;
      string s = GenRandomString( i + 10 );
      //cout << "doc->len: " << doc->length() << " test.len:" << test.length() << " addlen: " << s.length()<< endl;
      //printf("   insert:%s(%u) old:%s(%u)\n", s.c_str(), (uint)s.size(), old.c_str(), old.length());
      Insert( i, s );
      bool b = Compare();
      //cout << "   doc->len: " << doc->length() << " test.len:" << test.length() << endl;
      if ( !b ) {
        cout << "Failed index: " << i << endl;
        cout << "       old : " << old << endl;
        cout << "       inst: " << s << endl;
        char *p = new char[doc->length()+1];
        p[doc->length()] = 0;
        doc->get(0, (uchar*)p, doc->length());
        cout << "       doc : " << p << endl;
        cout << "       test: " << test << endl;
        delete [] p;
        Dump();
        exit(0);
      }
    }
    cout << "passed" << endl;

    // Delete
    cout << "## Delete " << endl;
    for ( int i = 0; i < length; i++ ) {
      for ( int k = 1; i + k < length; k++ ) {
        Init();
        //cout << "size; "<< test.length() << endl;
        //cout << " try index:" << i << " len:" << k <<endl;
        Delete( i, k );
        bool b = Compare();
        if ( !b ) {
          cout << "Failed i:" << i << " k:" << k << endl;
          Dump();
          exit(0);
        }
      }
    }
    cout << "passed" << endl;

    // Replace
    cout << "## Replace ";
    for ( int i = 0; i < length; i++ ) {
      for ( int k = 1; i + k < length; k++ ) {
        Init();
        // 置換のための文字数が適当すぎるかも
        string s = GenRandomString( k + 10 );
        string old = test;
        Replace( i, k, s.c_str(), s.size() );
        bool b = Compare();
        if ( !b ) {
          cout << "Failed i:" << i << " k:" << k << endl;
          cout << "       doc->len: " << doc->length() << " test.len:" << test.length() << endl;
          cout << "       old : " << old << endl;
          cout << "       inst: " << s << endl;
          char *p = new char[doc->length()+1];
          doc->get(0, (uchar*)p, doc->length());
          p[doc->length()] = 0;
          cout << "       doc : " << p << endl;
          cout << "       test: " << test << endl;
          delete [] p;
          Dump();
          exit(0);
        }
      }
    }
    cout << "passed" << endl;
    cout << endl;

    Clear();
  }


  void Init(bool b = false)
  {

#define TEST_MAX 6
    static const char *str[TEST_MAX] = {
      "01234",
      "<>?_}*{`~",
      "abc",
      "$%&'()",
      "44444",
      "555",
    };

    doc = new Document();
    doc->insert(0, (const uchar*)org_str, strlen(org_str));
    test = string(org_str);

    for ( int i = 0, max = count; i < max; i++ ) {
      Insert( 0, str[i] );
    }

    if ( b && count < TEST_MAX) ++count;

  }


  void Clear()
  {
    delete doc;
    doc = NULL;
    //doc = new doc( org );
    //test = str;
  }


  void Insert(int index, string ins)
  {
    Q_ASSERT( index <= doc->length());

    doc->insert( index, (uchar*)ins.c_str(), ins.size() );

    int i = min( index, (int)test.size() );
    test.insert( i, ins.c_str(), ins.length() );
    //cout << "#ins test: "<< test << " " << ins.length() << endl;
  }

  void Delete(int index, int length)
  {
    Q_ASSERT( length > 0 );
    Q_ASSERT( index + length <= doc->length() );

    doc->remove( index, (quint64)length );
    test.erase( index, length );
  }

  void Replace(int index, int length, const char *buf, int bufLength)
  {
    Q_ASSERT( length > 0 );
    Q_ASSERT( index + length <= doc->length() );
    
    //doc->Replace( index, length, buf, bufLength );
    doc->remove(index, (quint64)length);
    doc->insert(index, (uchar*)buf, bufLength);

    //test.replace( index, length, buf, bufLength );
    test.erase( index, length );
    test.insert( index, buf, bufLength );
  }

  bool Compare()
  {
    // Length
    if ( doc->length() != test.size() ) {
      cout << "Error[Compare]: length  piece: " << doc->length()
        << "  test:" << test.size() << endl;
      return false;
    }


    // Piece buffer
    if ( !doc->length() ) {
      return true;
    }
    char *p = new char[ doc->length() ];
    memset( p, 0, doc->length() );
    doc->get( 0, (uchar*)p, doc->length() );

    // Comparing
    if ( memcmp(p, test.c_str(), doc->length()) != 0  ) {
      delete [] p;
      cout << "Error[Compare]: compare" << endl;
      return false;
    }
    delete [] p;

    return true;
  }

  void DumpPieceString(const char *s)
  {
    cout << s;

    // Piece buffer
    if ( !doc->length() ) {
      return;
    }
    char *p = new char[ doc->length() + 1 ];
    memset( p, 0, doc->length() + 1 );
    doc->get( 0, (uchar*)p, doc->length() );
    
    cout << p << endl;

    delete [] p;
  }

  void DumpString(const char *s)
  {
    cout << s << test << endl;
  }

  void Dump()
  {
    cout << "Document::length = " << doc->length() << endl;
    cout << "String::length = " << test.length() << endl;
    DumpPieceString( "dump piece  : " );
    DumpString(      "std::string : " );
    //doc->Dump();
  }


  string GenRandomString(int length)
  {
    const char *t = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890";
    const int l = strlen(t);
    string s(length, 'X');
    for ( int i = 0; i < length; i++ ) {
      s[i] = static_cast<char>( t[ rand() & l ] );
    }
    return s;
  }



};


#if 0

int main(int argc, char* argv[])
{
  srand(127);
  Test t("aa");
  for ( int i = 0; i < TEST_MAX+1; i++ ) {
    t.TestUnit();
  }

  cout << "Test Finished" << endl;

  return 0;
}

#endif
