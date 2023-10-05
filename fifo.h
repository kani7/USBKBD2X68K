//
// リングバッファっぽいものを確保して8ビット専用FIFO風に使う
//    注意:255を超える深さのFIFOは扱えない
//        スレッドセーフなんて考慮に無い
//

class FIFO {
  public:
    // コンストラクタ
    //    引数  itmQty   キューに保持する要素の数
    FIFO(uint8_t itmQty) : _itmQty(itmQty) , _itmCount(0) {
      uint16_t size = sizeof(uint8_t) * _itmQty;
      _itmData = (uint8_t *)malloc(size);
      _idxPut = 0;
      _idxTake = 0;
      _itmCount = 0;
    }

    // デストラクタ
    //    コンストラクタで確保したメモリを開放する
    ~FIFO() {
      free(_itmData);
    }

    // オブジェクトの初期化
    //    初期化とは...
    void clear(void) {
      _idxPut = 0;
      _idxTake = 0;
      _itmCount = 0;
    }

    // キューの末尾に要素を格納する(push)
    //      引数    格納しようとしている要素
    //      戻り値  正常終了した時はtrue、それ以外はfalseを返す
    bool push(uint8_t itm) {
      if (isFull()) return(false);
      memcpy(_itmData + _idxPut, &itm, 1);
      _idxPut++;
      _itmCount++;
      if (_idxPut == _itmQty) _idxPut = 0;  //リングの終端に達した場合は先頭が次回格納先
      return(true);
    }

    // キューの先頭から要素を取り出す(pop)
    //      戻り値  キューの先頭に入っていた要素またはNULL
    //      注意    戻り値を見ても正常終了したか否かを判別できない
    //              (エラーでNULLが返ったのか、NULLをpushされていたのか区別できない)ので
    //              事前に isEmpty で確認する必要がある
    //      独り言  データを戻り値ではなくポインタ渡しにすれば良くね? という意見は認める
    uint8_t pop(void) {
      if (isEmpty()) return(NULL);
      uint8_t itm;
      memcpy(&itm, _itmData + _idxTake, 1);
      _idxTake++;
      _itmCount--;
      if (_idxTake == _itmQty) _idxTake = 0;  //リングの終端に達した場合は先頭が次回読み出し先
      return (itm);
    }

    // キューが完全に空か否かを確認する
    //      戻り値      全く何も入っていないならtrue、何か入っているならfalseを返す
    inline bool isEmpty(void) {
      return(_itmCount == 0);
    }

    // キューが満杯か否かを確認する
    //      戻り値      満杯ならtrue、空きがあるならfalseを返す
    inline bool isFull(void) {
      return (_itmCount != 0 && _itmCount == _itmQty);
    }

    // キューの現在の要素数を取得する
    //      戻り値      現在の要素数
    inline uint8_t size(void) {
      return _itmCount;
    }

  private:
    uint8_t   _itmQty;    // キュー内に保持できる要素数
    uint8_t*  _itmData;   // キューとして確保したメモリの先頭アドレスへのポインタ

    uint8_t   _itmCount;  // 現在キュー内に保持してる要素の数
    uint8_t   _idxPut;    // 次回pushされた時にメモリの何番目の位置に書くかを示す
    uint8_t   _idxTake;   // 次回popされた時にメモリの何番目を参照するかを示す
};