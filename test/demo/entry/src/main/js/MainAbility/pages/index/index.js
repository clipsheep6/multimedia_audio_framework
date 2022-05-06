import router from '@system.router';

export default {
    data: {
        title: ""
    },

    toPlayer() {
        router.push({
            uri: 'pages/player/player',
        })
    }
}
