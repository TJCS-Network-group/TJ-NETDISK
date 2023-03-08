<template>
  <div class="maindiv">
    <el-row type="flex" justify="center" style="margin: 0px 0 10px 0px">
      <div class="image">
        <img src="static/dog.png" class="pic" />
      </div>
    </el-row>
    <h1 class="title1">404</h1>
    <h2 class="title2">抱歉，您要访问的页面不存在！</h2>
    <el-row type="flex" justify="center" style="margin: 0px 0 10px 0px">
      <el-button
        @click="back"
        type="primary"
        class="backbutton"
        style="text-align: center"
        >返回</el-button
      >
    </el-row>
  </div>
</template>


<style scpoed>
html,
body,
#app,
.el-header {
  background-color: white;
}
.el-aside {
  background-color: white;
}
.title1 {
  font-size: 48px;
  text-align: center;
  font-weight: bold;
}
.title2 {
  font-size: 32px;
  text-align: center;
  font-weight: bold;
}
.maindiv {
  background-color: white;
  height: 100%;
}
.el-card {
  border-radius: 15px;
}
</style>

<script>
export default {
  name: "NotFound",
  components: {},
  methods: {
    back() {
      this.$router.back();
    },
  },
};
</script>
