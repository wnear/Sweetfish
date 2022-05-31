/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 */
#include "TootData.h"
#include <QtCore>

//自分の使用しているアカウントのidリスト
QByteArrayList TootData::static_owner_user_id_list;

TootCardData::TootCardData(const QJsonObject &target) {
  url = target["url"].toString();
  description = target["description"].toString();
  type = target["type"].toString();
  title = target["title"].toString();
  author_name = target["author_name"].toString();
  author_url = target["author_url"].toString();
  provider_name = target["provider_name"].toString();
  provider_url = target["provider_url"].toString();
  preview_url = target["image"].toString();
  if (preview_url.isEmpty()) {
    preview_url = target["embed_url"].toString();
  }
}

TootAccountData::TootAccountData(const QJsonObject &target) {
  if (target.isEmpty())
    return;
  id = target["id"].toString().toLatin1();
  user_name = target["username"].toString();
  display_name = target["display_name"].toString();
  if (display_name.isEmpty()) {
    display_name = user_name;
  }
  acct = target["acct"].toString();
  avatar = target["avatar"].toString(); //アイコン
  following_count = target["following_count"].toInt();
  followers_count = target["followers_count"].toInt();
  locked = target["locked"].toBool();
  description = target["note"].toString();
}

/*
 * 引数:なし
 * 戻値:TootAccountData(本来の投稿したユーザ情報)
 * 概要:投稿したユーザ情報を返す。ブーストなどはブースト元のアカウント情報を返す。
 */
const TootAccountData &TootData::getOriginalAccountData() const {
  if (m_reblog != nullptr)
    return reblog->m_account;
  return m_account;
}

TootRelationshipData::TootRelationshipData(const QJsonObject &target) {
  following = target["following"].toBool();
  followed = target["followed_by"].toBool();
  muting = target["muting"].toBool();
  blocking = target["blocking"].toBool();
}

/*
 * 引数:display_url(表示してるURLなど), full_url(完全なリンク)
 * 戻値:なし
 * 概要:エントリーを追加する。TootDataからの呼び出しを想定。
 */
void TootUrlData::addUrlPair(const QString &display_url,
                             const QString &full_url) {
  data.append(QPair<QString, QString>(display_url, full_url));
}

/*
 * 引数:index(0から始まるエントリー数)
 * 戻値:QString(表示してるURLなど)
 * 概要:指定されたindexの表示用URLを返す。indexが範囲外ならば空文字を返す。
 */
QString TootUrlData::getDisplayUrl(unsigned int index) const {
  return (index < data.count()) ? data.at(index).first : QString();
}

/*
 * 引数:index(0から始まるエントリー数)
 * 戻値:QString(完全なリンク)
 * 概要:指定されたindexの完全なリンクを返す。indexが範囲外ならば空文字を返す。
 */
QString TootUrlData::getFullUrl(unsigned int index) const {
  return (index < data.count()) ? data.at(index).second : QString();
}

TootMediaData::TootMediaData(const QJsonArray &target) {
  for (const QJsonValue &entry : target) {
    const QJsonObject entry_object = entry.toObject();
    TootMediaDataEntry s;
    s.type = entry_object["type"].toString();
    s.url = entry_object["url"].toString();
    s.remote_url = entry_object["remote_url"].toString();
    s.preview_url = entry_object["preview_url"].toString();
    s.text_url = entry_object["text_url"].toString();
    media_list.append(s);
  }
}

TootData::TootData(const QJsonObject &target) {
  if (target.find("id") == target.end())
    return;
  m_id = target["id"].toString().toLatin1();
  m_created_at.setTimeSpec(Qt::UTC);
  m_created_at =
      QDateTime::fromString(target["created_at"].toString(), Qt::ISODateWithMs);

  QJsonObject application_json = target["application"].toObject();
  m_application = QPair<QString, QString>(application_json["name"].toString(),
                                        application_json["website"].toString());
  m_uri = target["uri"].toString();
  m_url = target["url"].toString();
  analyzeContent(target["content"].toString());

  m_account = TootAccountData(target["account"].toObject());
  m_media = TootMediaData(target["media_attachments"].toArray());
  if (!target["card"].isNull()) {
    m_card = TootCardData(target["card"].toObject());
  }

  m_flag = 0;
  if (target["reblogged"].toBool()) {
    m_flag |= 1 << 0; //一般化
  }
  if (target["favourited"].toBool()) {
    m_flag |= 1 << 1;
  }
  if (static_owner_user_id_list.contains(m_account.getId())) {
    m_flag |= 1 << 2;
  }
  if (target["visibility"].toString() == "private") {
    m_flag |= 1 << 3;
  }
  if (target["visibility"].toString() == "direct") {
    m_flag |= 1 << 4;
  }

  if (QJsonValue reblog_status = target["reblog"]; reblog_status.isObject()) {
    QJsonObject reblog_object = reblog_status.toObject(); // constならいらない
    m_reblog = new TootData(reblog_object);
    m_media = m_reblog->getMediaData(); //扱いやすいように
    m_content = m_reblog->getContent();
  }
  //製作中
}

TootData::~TootData() { delete m_reblog; }

/*
 * 引数:なし
 * 戻値:bool(ブーストしていたらtrue、それ以外はfalse)
 * 概要:自分がブーストしているか返す。
 */
bool TootData::isBoosted() const { return m_flag & (1 << 0); }

/*
 * 引数:なし
 * 戻値:bool(お気に入りに登録していたらtrue、それ以外はfalse)
 * 概要:自分がお気に入りに登録しているか返す。
 */
bool TootData::isFavourited() const { return m_flag & (1 << 1); }

/*
 * 引数:なし
 * 戻値:bool(自分の投稿ならtrue、それ以外はfalse)
 * 概要:自分の投稿かどうかを返す。
 */
bool TootData::isTootOwner() const { return m_flag & (1 << 2); }

/*
 * 引数:なし
 * 戻値:bool(非公開の投稿ならtrue、それ以外はfalse)
 * 概要:非公開の投稿(フォローしてないと見れない)かどうかを返す。
 */
bool TootData::isPrivateToot() const { return m_flag & (1 << 3); }

/*
 * 引数:なし
 * 戻値:bool(ダイレクトメッセージならtrue、それ以外はfalse)
 * 概要:ダイレクトメッセージかどうかを返す。
 */
bool TootData::iSDirectMessage() const { return m_flag & (1 << 4); }

/*
 * 引数:なし
 * 戻値:QString(投稿に使用されたアプリケーションの名前)
 * 概要:viaソフトウェアの名前を返す。(空であることあり)
 */
QString TootData::getApplicationName() const { return m_application.first; }

/*
 * 引数:なし
 * 戻値:QString(投稿に使用されたアプリケーションのウェブページのURL)
 * 概要:viaソフトウェアのホームページを返す。(空であることあり)
 */
QString TootData::getApplicationSite() const { return m_application.second; }

/*
 * 引数:なし
 * 戻値:なし
 * 概要:MastodonはHTMLで本文投げてくるのでHTMLタグ取り除いてURL抽出などをする。絶対重い。
 */
void TootData::analyzeContent(QString c /*remove使うため参照ではない*/) {
  // spanとpを消す
  c.replace("</p><p>", "\n\n")
      .replace("<br>", "\n")
      .replace("<br />", "\n")
      .remove(QRegExp("<\\/?(span|p)[^>]*>"))
      .replace("&gt;", ">")
      .replace("&lt;", "<")
      .replace("&amp;", "&")
      .replace("&quot;", "\""); //見やすいように連結した。

  QRegularExpressionMatchIterator &&link_tags =
      QRegularExpression("<a[^>]* href=\"([^\"]*)\"[^>]*>([^<]*)<\\/a>")
          .globalMatch(c);
  while (link_tags.hasNext()) {
    QRegularExpressionMatch entry = link_tags.next();
    m_url_list.addUrlPair(entry.captured(2), entry.captured(1));
  }
  c.replace(QRegExp("<a[^>]* href=\"[^\"]*\"[^>]*>([^<]*)<\\/a>"), "\\1");
  m_content = c;
}

/*
 * 引数:id(使用しているアカウントのid)
 * 戻値:なし
 * 概要:トゥートが自分のものか判定するためのidを登録する。
 */
void TootData::addOwnerUserId(const QByteArray &id) {
  static_owner_user_id_list += id;
}

TootNotificationData::TootNotificationData(const QJsonObject &target) {
  if (target.find("id") == target.end())
    return;
  QString &&type_str = target["type"].toString();

  // type判定
  if (type_str == "mention") {
    m_type = Event::Mention;
  } else if (type_str == "reblog") {
    m_type = Event::Boost;
  } else if (type_str == "favourite") {
    m_type = Event::Favourite;
  } else if (type_str == "follow") {
    m_type = Event::Follow;
  } else {
    m_type = Event::NoEvent;
  }

  m_account = TootAccountData(target["account"].toObject());
  if (target["status"].isObject()) {
    m_status = TootData(target["status"].toObject());
  }
}
