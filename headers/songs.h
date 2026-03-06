#ifndef SONGS_H_
#define SONGS_H_

#include "addons/buzzerspeaker.h"

// セガの「セーガー」
// 「セ」の滞在時間を増やし、音の隙間を埋めて音量を稼いでいます
Song introSong{
    50, // テンポを少し速めにして、音の数を増やすことで密度を調整
    {
        PAUSE,
        G5, G5, G5, G5, G5, G5, G5, G5, G5, G5, G5, G5, G5, G5, G5, // 「セ」を12個に増加（密度UP）
        PAUSE,                                         // 休符を最小限にして音のキレを出す
        C5, C5, C5, C5, C5, C5, C5, C5, C5, C5, C5, C5, C5, C5, C5, PAUSE, PAUSE
    }
};

// サターン起動音
// C6付近が小さく感じる場合、その前の音を少し削り、C6の個数を増やして強調します
Song configModeSong{
    35, // 1単位をさらに短くし、より細かく制御
    {
        PAUSE, 
        F4, F4, F4, F4, F4, F4, F4, F4, PAUSE,
        
        // 前半：上昇アルペジオ（一気に駆け上がる）
        F4, F4, A4, A4, C5, C5, F5, F5, A5, A5, 
        C6, C6, C6, C6, C6, C6, C6, C6, C6, C6, // 小さく感じるC6の数を増やして強調
        
        // 独特の「溜め」：PAUSEを連続させることで、直前の高音との対比を作る
        PAUSE, PAUSE, PAUSE, PAUSE, PAUSE, 

        // 後半：広がる余韻（音量を稼ぐために少し長めに維持）
        C5, C5, C5, C5, C5, C5, C5, C5, C5, C5, C5, C5, C5, C5, C5, C5, C5, C5, PAUSE, PAUSE
    }
};

#endif
