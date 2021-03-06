#pragma once

#include <libdevcore/Common.h>
#include <libdevcore/SHA3.h>
#include <libdevcore/TrieCommon.h>
#include <libbrccore/Common.h>

#include <boost/filesystem/path.hpp>

namespace dev
{
class OverlayDB;

namespace brc
{
/**
 * Models the state of a single BrcdChain account.
 * Used to cache a portion of the full BrcdChain state. State keeps a mapping of Address's to
 * Accounts.
 *
 * Aside from storing the nonce and balance, the account may also be "dead" (where isAlive() returns
 * false). This allows State to explicitly store the notion of a deleted account in it's cache.
 * kill() can be used for this.
 *
 * For the account's storage, the class operates a cache. baseRoot() specifies the base state of the
 * storage given as the Trie root to be looked up in the state database. Alterations beyond this
 * base are specified in the overlay, stored in this class and retrieved with storageOverlay().
 * setStorage allows the overlay to be altered.
 *
 * The constructor allows you to create an one of a number of "types" of accounts. The default
 * constructor makes a dead account (this is ignored by State when writing out the Trie). Another
 * three allow a basic or contract account to be specified along with an initial balance. The fina
 * two allow either a basic or a contract account to be created with arbitrary values.
 */
class Account
{
public:
    /// Changedness of account to create.
    enum Changedness
    {
        /// Account starts as though it has been changed.
        Changed,
        /// Account starts as though it has not been changed.
        Unchanged
    };

    /// Construct a dead Account.
    Account() {}

    /// Construct an alive Account, with given endowment, for either a normal (non-contract) account
    /// or for a contract account in the conception phase, where the code is not yet known.
    Account(u256 _nonce, u256 _balance, Changedness _c = Changed)
      : m_isAlive(true), m_isUnchanged(_c == Unchanged), m_nonce(_nonce), m_balance(_balance)
    {}

	Account(u256 _nonce, u256 _balance, u256 _BRC, u256 _fbalance = 0, Changedness _c = Changed)
      : m_isAlive(true),
        m_isUnchanged(_c == Unchanged),
        m_nonce(_nonce),
        m_balance(_balance),
        m_BRC(_BRC),
		m_FBalance(_fbalance)
    {}

    /// Explicit constructor for wierd cases of construction or a contract account.
    Account(u256 _nonce, u256 _balance, h256 _contractRoot, h256 _codeHash, u256 _ballot,
        u256 _poll, u256 _BRC, u256 _FBRC, u256 _FBalance, Changedness _c, u256 _arrears,u256 _assetInjectStatus = 0)
      : m_isAlive(true),
        m_isUnchanged(_c == Unchanged),
        m_nonce(_nonce),
        m_balance(_balance),
        m_storageRoot(_contractRoot),
		m_codeHash(_codeHash), 
        m_ballot(_ballot), 
        m_poll(_poll),
        m_BRC(_BRC),
        m_FBRC(_FBRC),
        m_FBalance(_FBalance),
		m_assetInjectStatus(_assetInjectStatus),
        m_arrears(_arrears)
    {
        assert(_contractRoot);
    }

 Account(u256 _nonce, u256 _balance, h256 _contractRoot, h256 _codeHash, Changedness _c)
		:
		m_isAlive(true),
        m_isUnchanged(_c == Unchanged),
        m_nonce(_nonce),
        m_balance(_balance),
        m_storageRoot(_contractRoot),
        m_codeHash(_codeHash)
    {
        assert(_contractRoot);
    }


    /// Kill this account. Useful for the suicide opcode. Following this call, isAlive() returns
    /// false.
    void kill()
    {
        m_isAlive = false;
        m_storageOverlay.clear();
        m_storageOriginal.clear();
        m_codeHash = EmptySHA3;
        m_storageRoot = EmptyTrie;
        m_balance = 0;
        m_BRC = 0;
        m_FBRC = 0;
        m_FBalance = 0;
        m_nonce = 0;
        m_poll = 0;
        m_ballot = 0;
		m_assetInjectStatus = 0;
        m_arrears = 0;
        m_voteData.clear();
		m_BlockReward.clear();
        changed();
    }

    /// @returns true iff this object represents an account in the state. Returns false if this
    /// object represents an account that should no longer exist in the trie (an account that never
    /// existed or was suicided).
    bool isAlive() const { return m_isAlive; }

    /// @returns true if the account is unchanged from creation.
    bool isDirty() const { return !m_isUnchanged; }

    void untouch() { m_isUnchanged = true; }

    /// @returns true if the nonce, balance and code is zero / empty. Code is considered empty
    /// during creation phase.
    bool isEmpty() const { return nonce() == 0 && balance() == 0 && codeHash() == EmptySHA3 && BRC() == 0 && FBalance() == 0 && FBRC() == 0 && voteData().empty() && m_BlockReward.size() == 0;  }

    /// @returns the balance of this account.
    u256 const& balance() const { return m_balance; }


    u256 const& BRC() const { return m_BRC; }

    u256 const& FBRC() const { return m_FBRC; }

    u256 const& FBalance() const { return m_FBalance; }

    u256 const& arrears() const {return m_arrears;}
    /// Increments the balance of this account by the given amount.
    void addBalance(u256 _value)
    {
        m_balance += _value;
        changed();
    }

    // add BRC
    void addBRC(u256 _value)
    {
        m_BRC += _value;
        changed();
    }


	//add FBRC
    void addFBRC(u256 _value)
	{
		m_FBRC += _value;
        changed();
	}

    void addFBalance(u256 _value)
	{
		m_FBalance += _value;
        changed();
	}

    void addArrears(u256 _value)
    {
        m_arrears += _value;
        changed();
    }

    // Acounts own ballot
    u256 const& ballot() const { return m_ballot; }
    void addBallot(u256 _value) { m_ballot += _value; changed(); }

    // Acounts have poll by other
    u256 const& poll() const { return m_poll; }
    void addPoll(u256 _value) { m_poll += _value; changed(); }

    /// @returns the nonce of the account.
    u256 nonce() const { return m_nonce; }

	u256 assetInjectStatus() const{ return m_assetInjectStatus; }
	void setAssetInjectStatus() { m_assetInjectStatus = 1; }
    /// Increment the nonce of the account by one.
    void incNonce()
    {
        ++m_nonce;
        changed();
    }

    /// Set nonce to a new value. This is used when reverting changes made to
    /// the account.
    void setNonce(u256 const& _nonce)
    {
        m_nonce = _nonce;
        changed();
    }

    /// @returns the root of the trie (whose nodes are stored in the state db externally to this
    /// class) which encodes the base-state of the account's storage (upon which the storage is
    /// overlaid).
    h256 baseRoot() const
    {
        assert(m_storageRoot);
        return m_storageRoot;
    }

    /// @returns account's storage value corresponding to the @_key
    /// taking into account overlayed modifications
    u256 storageValue(u256 const& _key, OverlayDB const& _db) const
    {
        auto mit = m_storageOverlay.find(_key);
        if (mit != m_storageOverlay.end())
            return mit->second;

        return originalStorageValue(_key, _db);
    }

    /// @returns account's original storage value corresponding to the @_key
    /// not taking into account overlayed modifications
    u256 originalStorageValue(u256 const& _key, OverlayDB const& _db) const;

    /// @returns the storage overlay as a simple hash map.
    std::unordered_map<u256, u256> const& storageOverlay() const { return m_storageOverlay; }

    /// Set a key/value pair in the account's storage. This actually goes into the overlay, for
    /// committing to the trie later.
    void setStorage(u256 _p, u256 _v)
    {
        m_storageOverlay[_p] = _v;
        changed();
    }

    /// Empty the storage.  Used when a contract is overwritten.
    void clearStorage()
    {
        m_storageOverlay.clear();
        m_storageOriginal.clear();
        m_storageRoot = EmptyTrie;
        changed();
    }

    /// Set the storage root.  Used when clearStorage() is reverted.
    void setStorageRoot(h256 const& _root)
    {
        m_storageOverlay.clear();
        m_storageOriginal.clear();
        m_storageRoot = _root;
        changed();
    }

    /// @returns the hash of the account's code.
    h256 codeHash() const { return m_codeHash; }

    bool hasNewCode() const { return m_hasNewCode; }

    /// Sets the code of the account. Used by "create" messages.
    void setCode(bytes&& _code);

    /// Specify to the object what the actual code is for the account. @a _code must have a SHA3
    /// equal to codeHash().
    void noteCode(bytesConstRef _code)
    {
        assert(sha3(_code) == m_codeHash);
        m_codeCache = _code.toBytes();
    }

    /// @returns the account's code.
    bytes const& code() const { return m_codeCache; }

    // VoteDate 投票数据
    u256 voteAll()const { u256 vote_num = 0; for(auto val : m_voteData) vote_num += val.second; return vote_num; }
    u256 vote(Address const& _id) const { auto ret = m_voteData.find(_id); if(ret == m_voteData.end()) return 0; return ret->second; }
    void addVote(std::pair<Address, u256> _votePair);
    std::unordered_map<Address, u256> const& voteData() const { return m_voteData; }
    void setVoteDate(std::unordered_map<Address, u256> const& _vote) { m_voteData.clear(); m_voteData.insert(_vote.begin(), _vote.end()); }
    // 系统管理竞选人/验证人
	void manageSysVote(Address const& _otherAddr, bool _isLogin, u256 _tickets);


	void addBlockRewardRecoding(std::pair<u256, u256> _pair);

	void setBlockReward(std::unordered_map<u256, u256> const& _blockReward) { m_BlockReward.clear(); m_BlockReward.insert(_blockReward.begin(), _blockReward.end()); }
	std::unordered_map<u256, u256> const& blockReward() const { return m_BlockReward; }

private:
    /// Note that we've altered the account.
    void changed() { m_isUnchanged = false; }

    /// Is this account existant? If not, it represents a deleted account.
    bool m_isAlive = false;

    /// True if we've not made any alteration to the account having been given it's properties
    /// directly.
    bool m_isUnchanged = false;

    /// True if new code was deployed to the account
    bool m_hasNewCode = false;

    /// Account's nonce.
    u256 m_nonce;


    /// Account's balance.
    u256 m_balance = 0;

    /// The base storage root. Used with the state DB to give a base to the storage.
    /// m_storageOverlay is overlaid on this and takes precedence for all values set.
    h256 m_storageRoot = EmptyTrie;

    /** If c_contractConceptionCodeHash then we're in the limbo where we're running the
     * initialisation code. We expect a setCode() at some point later. If EmptySHA3, then m_code,
     * which should be empty, is valid. If anything else, then m_code is valid iff it's not empty,
     * otherwise, State::ensureCached() needs to be called with the correct args.
     */
    h256 m_codeHash = EmptySHA3;

    // 自己拥有的票数 可以投给竞选节点的票数
    u256 m_ballot = 0;

    // 在投票开启中自己的得到的票数 不能使用
    u256 m_poll = 0;

	// Account's BRC
    u256 m_BRC = 0;

    // Account's FBRC
    u256 m_FBRC = 0;
    u256 m_FBalance = 0;

	u256 m_assetInjectStatus = 0;
    u256 m_arrears = 0;
    /* dpos 投票数据
       Address : 投票目标 size_t: 票数
       当该Account 为系统预制地址表表示为 竞选人集合
    */
    std::unordered_map<Address, u256> m_voteData;

	std::unordered_map<u256, u256> m_BlockReward;

    /// The map with is overlaid onto whatever storage is implied by the m_storageRoot in the trie.
    mutable std::unordered_map<u256, u256> m_storageOverlay;

    /// The cache of unmodifed storage items
    mutable std::unordered_map<u256, u256> m_storageOriginal;

    /// The associated code for this account. The SHA3 of this should be equal to m_codeHash unless
    /// m_codeHash equals c_contractConceptionCodeHash.
    bytes m_codeCache;

    /// Value for m_codeHash when this account is having its code determined.
    static const h256 c_contractConceptionCodeHash;
};

class AccountMask
{
public:
    AccountMask(bool _all = false)
      : m_hasBalance(_all), m_hasNonce(_all), m_hasCode(_all), m_hasStorage(_all)
    {}

    AccountMask(bool _hasBalance, bool _hasNonce, bool _hasCode, bool _hasStorage,
        bool _shouldNotExist = false)
      : m_hasBalance(_hasBalance),
        m_hasNonce(_hasNonce),
        m_hasCode(_hasCode),
        m_hasStorage(_hasStorage),
        m_shouldNotExist(_shouldNotExist)
    {}

    bool allSet() const { return m_hasBalance && m_hasNonce && m_hasCode && m_hasStorage; }
    bool hasBalance() const { return m_hasBalance; }
    bool hasNonce() const { return m_hasNonce; }
    bool hasCode() const { return m_hasCode; }
    bool hasStorage() const { return m_hasStorage; }
    bool shouldExist() const { return !m_shouldNotExist; }

private:
    bool m_hasBalance;
    bool m_hasNonce;
    bool m_hasCode;
    bool m_hasStorage;
    bool m_shouldNotExist = false;
};

using AccountMap = std::unordered_map<Address, Account>;
using AccountMaskMap = std::unordered_map<Address, AccountMask>;

class PrecompiledContract;
using PrecompiledContractMap = std::unordered_map<Address, PrecompiledContract>;

AccountMap jsonToAccountMap(std::string const& _json, u256 const& _defaultNonce = 0,
    AccountMaskMap* o_mask = nullptr, PrecompiledContractMap* o_precompiled = nullptr,
    const boost::filesystem::path& _configPath = {});
}  // namespace brc
}  // namespace dev

