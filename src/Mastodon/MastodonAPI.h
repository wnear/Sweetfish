/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 */
#pragma once

#include "../Network/Network.h"
#include "../Setting/Setting.h"
#include <QStringList>

class QNetworkReply;
class QNetworkRequest;
class Network;
class Setting;

class MastodonAPI {
public:
  explicit MastodonAPI();
  MastodonAPI(const MastodonAPI &other);
  virtual ~MastodonAPI();

  //認証関係
  QNetworkReply *registerApp(const QString &domain);
  QUrl getAuthorizeUrl(const QString &domain, const QString &client_id) const;
  QNetworkReply *requestAccessToken(const QString &domain,
                                    const QByteArray &client_id,
                                    const QByteArray &client_secret,
                                    const QString &authorization_token_code);
  void setAccessToken(const QByteArray &token);
  void setInstanceDomain(const QString &instance_domain);
  void setUserId(const QByteArray &id);
  QByteArray getUserId() const;
  //トゥート関係
  QNetworkReply *requestToot(
      const QString &message,
      const QByteArray &media_id = QByteArray() /*ポインタのほうがいいかな...*/,
      const QByteArray &reply_id = QByteArray());
  QNetworkReply *requestDeleteToot(const QByteArray &id);
  QNetworkReply *requestBoost(const QByteArray &id);
  QNetworkReply *requestFavourite(const QByteArray &id);
  //取得関係
  QNetworkReply *requestHomeTimeLine(const QByteArray &since_id = QByteArray());
  QNetworkReply *requestListTimeLine(const QByteArray &list_id, const QByteArray &since_id = QByteArray());
  QNetworkReply *requestUserStream();
  QNetworkReply *requestListStream(const QByteArray &list_id);
  //メディア関係
  QNetworkReply *requestMediaUpload(QIODevice &data,
                                    const QByteArray &mime_type);
  //リスト関係
  QNetworkReply *requestGetLists();
  //ユーザ関係
  QNetworkReply *requestCurrentAccountInfo();
  QNetworkReply *requestUserStatuses(const QByteArray &user_id);
  QNetworkReply *requestUserRelationship(const QByteArray &user_id);
  QNetworkReply *requestFollow(const QByteArray &user_id);
  QNetworkReply *requestUnfollow(const QByteArray &user_id);
  QNetworkReply *requestBlock(const QByteArray &user_id);
  QNetworkReply *requestUnblock(const QByteArray &user_id);

private:
  /*汎用関数。あくまでMastodonAPIクラスから呼ぶもので他のクラス(UIなど)からはこれを呼ばず専用の関数を作る。*/
  QNetworkReply *get(QNetworkRequest &req);
  QNetworkReply *post(QNetworkRequest &req, const QByteArray &data);
  QNetworkReply *del(QNetworkRequest &req);
  QNetworkReply *upload(QNetworkRequest &req, const QByteArrayList &info,
                        QIODevice &data);

  QByteArray user_id;
  QByteArray access_token;
  QString domain;
  Network net;
};
