/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 */
#pragma once

#include <QDateTime>
#include <QJsonObject>
#include <QString>
#include <QVector>

class TootAccountData {
public:
  TootAccountData(){};
  explicit TootAccountData(const QJsonObject &target);

  bool isEmpty() const { return id.isEmpty(); };
  bool isLocked() const { return locked; };

  // getter
  QByteArray getId() const { return id; };
  QString getUserName() const { return user_name; };
  QString getDisplayName() const { return display_name; };
  QString getAcct() const { return acct; };
  QString getAvatar() const { return avatar; };
  QString getDescription() const { return description; };
  unsigned int getFollowingCount() const { return following_count; };
  unsigned int getFollowersCount() const { return followers_count; };

private:
  QByteArray id;
  QString user_name;  //スクリーンネーム(ABC)
  QString acct;       //インスタンス名を含むユーザ名
  QString display_name;
  QString avatar;  //ユーザアイコン
  QString description;
  unsigned int following_count;
  unsigned int followers_count;
  bool locked;
  // bool following;
};

class TootRelationshipData {
public:
  TootRelationshipData(){};
  explicit TootRelationshipData(const QJsonObject &target);

  bool isfollowing() const { return following; };
  bool isfollowed() const { return followed; };
  bool ismuting() const { return muting; };
  bool isblocking() const { return blocking; };

private:
  bool following = false;
  bool followed = false;
  bool muting = false;
  bool blocking = false;
};

class TootMediaDataEntry {
public:
  // getter
  QString getType() const { return type; };
  QString getUrl() const { return url; };
  QString getRemoteUrl() const { return remote_url; };
  QString getPreviewUrl() const { return preview_url; };
  QString getTextUrl() const { return text_url; };

private:
  QString type;
  QString url;
  QString remote_url;
  QString preview_url;
  QString text_url;

  friend class TootMediaData;
};

class TootCardData {
public:
  TootCardData(){};
  explicit TootCardData(const QJsonObject &target);
  QString getUrl() const { return url; };
  QString getTitle() const { return title; };
  QString getType() const { return type; }
  QString getAuthorName() const { return author_name; };
  QString getAuthorUrl() const { return author_url; };
  QString getProviderName() const { return provider_name; };
  QString getProviderUrl() const { return provider_url; };
  QString getPreviewUrl() const { return preview_url; };
  QString getDescription() const { return description; };

private:
  QString url;
  QString title;
  QString type;
  QString author_name;
  QString author_url;
  QString preview_url;
  QString description;
  QString provider_name;
  QString provider_url;
};

class TootMediaData {
public:
  TootMediaData(){};
  explicit TootMediaData(const QJsonArray &target);
  unsigned int size() const { return media_list.size(); };
  TootMediaDataEntry getEntry(unsigned int index) const { return media_list.at(index); };

private:
  QVector<TootMediaDataEntry> media_list;
};

class TootUrlData {
public:
  TootUrlData(){};
  void addUrlPair(const QString &display_url, const QString &full_url);
  unsigned int count() const { return data.count(); };
  unsigned int size() const { return count(); };
  QString getDisplayUrl(unsigned int index) const;
  QString getFullUrl(unsigned int index) const;

private:
  QList<QPair<QString, QString>> data;
};

class TootData {
public:
  TootData(){};
  explicit TootData(const QJsonObject &target);
  virtual ~TootData();

  bool isEmpty() const { return m_id.isEmpty(); };
  bool isBoosted() const;
  bool isFavourited() const;
  bool isTootOwner() const;
  bool isPrivateToot() const;
  bool iSDirectMessage() const;

  // getter
  QByteArray getId() const { return m_id; };
  QString getApplicationName() const;
  QString getApplicationSite() const;
  const TootAccountData &getAccountData() const { return m_account; };
  const TootAccountData &getOriginalAccountData() const;
  TootData *getBoostedData() const { return m_reblog; };
  const TootUrlData &getUrlData() const { return m_url_list; };
  const TootMediaData &getMediaData() const { return m_media; };
  const TootCardData &getCardData() const { return m_card; };
  QDateTime getDateTime() const { return m_created_at; };
  QString getContent() const { return m_content; };

  // setter
  static void addOwnerUserId(const QByteArray &id);

private:
  QByteArray m_id;
  QDateTime m_created_at;
  QString m_uri;
  QString m_url;
  QString m_content;
  QPair<QString, QString> m_application;  // via
  qint64 m_flag;

  TootAccountData m_account;
  TootUrlData m_url_list;
  TootMediaData m_media;
  TootCardData m_card;
  TootData *m_reblog = nullptr;

  void analyzeContent(QString content);

  static QByteArrayList static_owner_user_id_list;  //あまりいい方法ではない気もする...
};

class TootNotificationData {
public:
  enum Event {  //使いそうなやつだけ
    NoEvent,
    Mention,
    Boost,
    Favourite,
    // Unfavorite,
    // QuotedTweet,//引用ツイート
    Follow,
    // Unfollow,
    // ListCreated,
    // ListDestroyed,
    // ListUpdated,
    // ListMemberAdded,
    // ListMemberRemoved
  };
  TootNotificationData(){};
  explicit TootNotificationData(const QJsonObject &target);

  bool isEmpty() const { return (m_type == Event::NoEvent); };

  // getter
  Event getType() const { return m_type; };
  const TootAccountData &getAccount() const { return m_account; };
  const TootData &getStatus() const { return m_status; };

private:
  TootAccountData m_account;
  TootData m_status;
  Event m_type;
};
