const assert = require('assert');

const PROFILE_WASM_PATH = './compiled/profile.wasm';
const PROFILE_ABI_PATH = './compiled/profile.abi';

describe("Test profile contract", function (eoslime) {

    // Increase mocha(testing framework) time, otherwise tests fails
    this.timeout(50000);

    let profileContract;
    let profileUser1;
    let profileUser2;

    const userprofile1 = {
      nickname: 'John Doe',
      avatar: 'https://johnfdoe.com/avatars/JohnDoe.jpg',
      website: 'https://johnfdoe.com',
      locale: 'en_US',
      metadata: '{}'
    };
    const userprofile2 = {
      nickname: 'Jane Doe',
      avatar: 'https://johnfdoe.com/avatars/JaneDoe.jpg',
      website: 'https://johnfdoe.com/wife',
      locale: 'en_US',
      metadata: '{}'
    };

    before(async () => {
        const accounts = await eoslime.Account.createRandoms(2);
        profileUser1 = accounts[0];
        profileUser2 = accounts[1];

        // Print accounts
        console.log('profile user 1:', profileUser1.name);
        console.log('profile user 2:', profileUser2.name);

        /*
            CleanDeployer creates for you a new account behind the scene
            on which the contract code is deployed

            Note! CleanDeployer always deploy the contract code on a new fresh account

            You can access the contract account as -> tokenContract.executor
        */
        profileContract = await eoslime.Contract.deploy(PROFILE_WASM_PATH, PROFILE_ABI_PATH);

        // print contract account
        console.log('contract user:', profileContract.executor.name);

        // Add `eosio.code` permission to be able to use deferred transactions
        await profileContract.executor.addPermission('eosio.code');
    });

    it("update John Doe profile", async () => {
        let tx = await profileContract.update(profileUser1.name, userprofile1.nickname, userprofile1.avatar, userprofile1.website,
                                              userprofile1.locale, userprofile1.metadata, {from: profileUser1});

        console.log(tx.processed.action_traces[0].console);

        let profileData = await profileContract.profiles.equal(profileUser1.name).find();

        assert.notEqual(profileData[0],       undefined,              "No user profile defined");
        assert.equal(profileData[0].user,     profileUser1.name,      "Incorrect user");
        assert.equal(profileData[0].nickname, userprofile1.nickname,  "Incorrect nickname");
        assert.equal(profileData[0].avatar,   userprofile1.avatar,    "Incorrect avatar");
        assert.equal(profileData[0].website,  userprofile1.website,   "Incorrect website");
        assert.equal(profileData[0].locale,   userprofile1.locale,    "Incorrect locale");
        assert.equal(profileData[0].metadata, userprofile1.metadata,  "Incorrect metadata");
    });

    it("update Jane Doe profile", async () => {
        let tx = await profileContract.update(profileUser2.name, userprofile2.nickname, userprofile2.avatar, userprofile2.website,
                                              userprofile2.locale, userprofile2.metadata, {from: profileUser2});

        console.log(tx.processed.action_traces[0].console);

        let profileData = await profileContract.profiles.equal(profileUser2.name).find();

        assert.notEqual(profileData[0],       undefined,              "No user profile defined");
        assert.equal(profileData[0].user,     profileUser2.name,      "Incorrect user");
        assert.equal(profileData[0].nickname, userprofile2.nickname,  "Incorrect nickname");
        assert.equal(profileData[0].avatar,   userprofile2.avatar,    "Incorrect avatar");
        assert.equal(profileData[0].website,  userprofile2.website,   "Incorrect website");
        assert.equal(profileData[0].locale,   userprofile2.locale,    "Incorrect locale");
        assert.equal(profileData[0].metadata, userprofile2.metadata,  "Incorrect metadata");
    });

    it("wait 15 seconds", (done) => {
        this.timeout(15000);
        setTimeout(done, 15000);
    });

    it("John Doe check count over 10", async () => {
        let profileData = await profileContract.profiles.equal(profileUser1.name).find();

        assert(profileData[0].count >= 10, "Not at least 10 yet");
    });

    it("Jane Doe check count still 0", async () => {
        let profileData = await profileContract.profiles.equal(profileUser2.name).find();

        assert.equal(profileData[0].count, '0.00000000000000000', "Not 0 still");
    });

});
