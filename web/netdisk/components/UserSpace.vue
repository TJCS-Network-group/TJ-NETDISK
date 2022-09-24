<template>
	<div>
  <div id="dirmenu" v-show="DirMenuVisible" class="menu">
  <div class="contextmenu_item" @click="removeMenu();renameForm.name=selectRow.name;renameForm.visible=true">重命名</div>
	<div class="contextmenu_item" @click="removeMenu();deleteFunc()">删除文件夹</div>
	<!-- <div class="contextmenu_item" @click="removeMenu();downloadFunc()">下载文件夹</div> -->
	<div class="contextmenu_item" @click="removeMenu();openCopyForm()">复制</div>
	<div class="contextmenu_item" @click="removeMenu();openMoveForm()">移动</div>
	<!-- <div class="contextmenu_item" @click="removeMenu();openShareForm()">共享</div> -->
  </div>
  <div id="filemenu" v-show="FileMenuVisible" class="menu">
  <div class="contextmenu_item" @click="removeMenu();renameForm.name=selectRow.name;renameForm.visible=true">重命名</div>
	<div class="contextmenu_item" @click="removeMenu();deleteFunc()">删除文件</div>
	<div class="contextmenu_item" @click="removeMenu();downloadFunc()">下载文件</div>
	<div class="contextmenu_item" @click="removeMenu();openCopyForm()">复制</div>
	<div class="contextmenu_item" @click="removeMenu();openMoveForm()">移动</div>
	<!-- <div class="contextmenu_item" @click="removeMenu();openShareForm()">共享</div> -->
  </div>

    <el-dialog width="30%" v-model="newDirForm.visible" title="新建目录">
    <el-form>
      <el-form-item label="新建目录" >  
        <el-input 
          rows=8 
          placeholder="请输入目录名" 
          v-model="newDirForm.name" 
          maxlength=20
          resize="vertical"
          autocomplete="off" />
      </el-form-item>
    </el-form>
    <template #footer>
      <span class="dialog-footer">
        <el-button @click="newDirForm.visible = false">取消</el-button>
        <el-button type="primary" @click="newDir"
          >确认</el-button
        >
      </span>
    </template>
  </el-dialog>

	<el-dialog width="40%" v-model="copyForm.visible" title="复制到">
		<div class="el-dialog-div">
		<el-container>
			<el-main>
				<el-tree 
			ref="copyFileTree"
      class="file-tree"
      :data="fileTree" 
      node-key="id"
      empty-text=""
			:expand-on-click-node="false"
      @node-expand="(data,node,t)=>{data.isopen=true;}"
      @node-collapse="(data,node,t)=>{data.isopen=false;}">
		<template #default="{ node, data }">
		<span  class="custom-tree-node">
					<el-icon v-if="data.children && data.isopen"><FolderOpened /></el-icon>
					<el-icon v-else-if="data.children && !data.isopen"><Folder/></el-icon>
					<el-icon v-else><Document /></el-icon>
				<i v-if="data.children && !data.isopen" class="el-icon-folder"></i>
				<i v-else-if="data.children && data.isopen" class="el-icon-folder-opened"></i>
				<i v-else class="el-icon-document"></i>
				<span>{{ node.label }}</span>
		</span>
		</template>
		</el-tree>
			</el-main>
			<el-footer>
				<span class="dialog-footer">
        <el-button @click="closeCopyForm()">取消</el-button>
        <el-button type="primary" @click="copyFunc()"
          >确认</el-button
        >
    		</span>
			</el-footer>
		</el-container>
		</div>

	</el-dialog>


	<el-dialog width="40%" v-model="moveForm.visible" title="移动到">
		<div class="el-dialog-div">
		<el-container>
			<el-main>
				<el-tree 
			ref="moveFileTree"
      class="file-tree"
      :data="fileTree" 
      node-key="id"
      empty-text=""
			:expand-on-click-node="false"
      @node-expand="(data,node,t)=>{data.isopen=true;}"
      @node-collapse="(data,node,t)=>{data.isopen=false;}">
		<template #default="{ node, data }">
		<span  class="custom-tree-node">
					<el-icon v-if="data.children && data.isopen"><FolderOpened /></el-icon>
					<el-icon v-else-if="data.children && !data.isopen"><Folder/></el-icon>
					<el-icon v-else><Document /></el-icon>
				<i v-if="data.children && !data.isopen" class="el-icon-folder"></i>
				<i v-else-if="data.children && data.isopen" class="el-icon-folder-opened"></i>
				<i v-else class="el-icon-document"></i>
				<span>{{ node.label }}</span>
		</span>
		</template>
		</el-tree>
			</el-main>
			<el-footer>
				<span class="dialog-footer">
        <el-button @click="closeMoveForm()">取消</el-button>
        <el-button type="primary" @click="moveFunc()"
          >确认</el-button
        >
    		</span>
			</el-footer>
		</el-container>
		</div>

	</el-dialog>


	<!-- <el-dialog width="60%" v-model="this.shareForm.visible" title="分享">
		<div class="el-dialog-div-share">
				  <el-input
						v-model="shareForm.shareURL"
						:rows="2"
						type="textarea"
						:readonly="true"
					/>
					<br/>
					<br/>
				<span class="dialog-footer">
        <el-button @click="closeShareForm()">关闭</el-button>
        <el-button type="primary" @click="copyShareURL"
          >复制链接</el-button
        >
    		</span>
		</div>

	</el-dialog> -->


	

	<el-dialog width="30%" v-model="renameForm.visible" title="重命名">
    <el-form>
      <el-form-item label="重命名" >  
        <el-input 
          rows=8 
          placeholder="" 
          v-model="renameForm.name" 
          maxlength=20
          resize="vertical"
          autocomplete="off" />
      </el-form-item>
    </el-form>
    <template #footer>
      <span class="dialog-footer">
        <el-button @click="renameForm.visible = false">取消</el-button>
        <el-button type="primary" @click="renameFunc()">确认</el-button>
      </span>
    </template>
  </el-dialog>

		<el-dialog
			title="上传文件"
			v-model="uploadForm.visible"
			width="30%"
    >    
    <el-upload
      class="upload-demo"
			ref="upload"
			:show-file-list=false
			:action="uploadURL"
      :on-success="handleUploadSucess"
      multiple
      :limit = "10"
      :before-upload="beforeUpload"
      :on-exceed="handleExceed"
      :on-error="handleFail"
      :file-list="uploadForm.fileList"
			:http-request="uploadFile">
      <el-button  type="primary">点击上传</el-button>

      <!-- <template #tip>
        <div class="el-upload__tip">
          file size no more than 1M<br>file num no more than 10
        </div>
      </template> -->

    </el-upload>

    <template #footer>
    <span class="dialog-footer">
      <el-button @click="uploadForm.visible = false">取消</el-button>
      <el-button type="primary" @click="uploadForm.visible = false">确认</el-button>
    </span>
    </template>
  </el-dialog>


		<el-container>
			<el-header style="border-bottom: solid rgba(219, 219, 219, 0.874);">
				<Header></Header>
			</el-header>
			<el-container>
			<el-header height="15px" style="margin-top:10px">
        <div>
      <el-button 
        style="margin-left:5px" 
        @click="newDirForm.visible=true"
        circle>
        <el-icon ><FolderAdd/></el-icon>
      </el-button>
      <el-button 
        style="margin-left:5px" 
        @click="uploadForm.visible=true;"
        circle>
        <el-icon><Upload/></el-icon>
      </el-button>

      
      <el-button 
        style="margin-left:5px" 
        @click="refreshPath()"
        circle>
        <el-icon><RefreshRight/></el-icon>
      </el-button>
      <el-button 
        style="margin-left:5px;margin-right:20px;" 
        @click="jumpPath(Math.max(path_list.length-2,0))"
        circle>
        <el-icon><ArrowLeft/></el-icon>
      </el-button>

						<el-button-group>
            <el-button 
						v-for="(path, index) in path_list" 
						:key="index" 
						type=""	
						size="default" 
						@click="jumpPath(index)">
              {{ path.name }}
            </el-button>
						</el-button-group>

        </div>
			</el-header>
			<el-main>
				<el-table
					:data="file_list"
					style="width: 100%"
					max-height="750"
					@row-contextmenu="rightClick"
				>
					<el-table-column
						label="名称"
						width="400"
					>
						<template #default="scope">
							<div>
								<el-icon><Folder v-if="scope.row.type=='1'"/><Document v-else /></el-icon>
								<el-button
                  v-if="scope.row.type=='1'"
									type="text"
									@click="jumpDir(scope.row.id,scope.row.name)"
								>
									{{ scope.row.name }}
								</el-button>
                <el-button
                  v-else
									type="text"
									@click="jumpFile(scope.row.id,scope.row.name)"
								>
									{{ scope.row.name }}
								</el-button>
							</div>
						</template>
					</el-table-column>
					<el-table-column
						prop="lastchange"
						label="修改日期"
						width="180"
					/>
					<el-table-column
						label="大小"
					>
					<template #default="scope">
							<div>
								<span
                  v-if="scope.row.type==1"
								>
									-
								</span>
								<span v-else>
									{{ scope.row.fsize>1024*1024?(scope.row.fsize/1024/1024).toFixed(2)+'MB':scope.row.fsize>1024?(scope.row.fsize/1024).toFixed(2)+'KB':scope.row.fsize.toString()+'B' }}
								</span>
							</div>
						</template>
					</el-table-column>
				</el-table>
				
			</el-main>
		</el-container>
		</el-container> 
		


	</div>
</template>
  
<script >
const Header = Vue.defineAsyncComponent(() => loadModule('./components/Header.vue', componentOptions))
const { ArrowDown,CirclePlus, Plus, Minus, FolderAdd,Upload,RefreshRight,ArrowLeft,Folder,FolderOpened,Document,Check } = ElementPlusIconsVue
const { reactive, toRefs } = Vue
const { ElMessage, ElMessageBox } = ElementPlus;
// import Cookies from 'js-cookie'
// import SparkMD5 from 'spark-md5'
console.log(Header)

let uploadParam={
	pid:0,
	axios:null,
}

export default {
	components: {
		Minus,
		Plus,
		Check,
		FolderAdd,
    Upload,
    RefreshRight,
    ArrowLeft,
		reactive,
		toRefs,
		CirclePlus,
		ArrowDown,
		ElMessageBox,
    ElMessage,
		Header,
		Folder,
		FolderOpened,
		Document,
	},
	el: '#app',
	name: 'userspace',
	data() {
		return {
			uid: '',
			email: '132',
			Username: '',

			imageUrl: '',
			file_list: [],
      path_list: [],
      newDirForm:{
        visible:false,
        name:'',
      },
      uploadForm:{
        visible :false,
        name:'',
        fileList: [],
      },
			renameForm:{
        visible :false,
        name:'',
      },
			copyForm:{
        visible :false,
      },
			moveForm:{
        visible :false,
      },
			shareForm:{
        visible :false,
				shareURL: null,
      },
			uploadURL: '',
			DirMenuVisible: false,
			FileMenuVisible: false,
			selectRow: null,
			fileTree: [],
			selectNode: null,
			shareClipboard: null,
		}
	},
	created() {
		// axios.get('/api/myinfo/', {}).then((response) => {
		// 	console.log()
		// 	this.email = response.data.data.email
		// 	this.uid = response.data.data.uid
		// 	this.imageUrl =
		// 		'/static/avatar/' + this.uid + '.jpg' + `?${Date.now()}`
		// })
		axios.get('/api/get_root_id', {}).then((response) => {
			console.log(response)
			this.root_id = response.data.data.root_id
			axios.get('/api/filesystem/get_dir',{params:{dir_id:this.root_id}}).then((response)=>{
				console.log(response)
				if(response.data.statusCode==200){
					this.file_list=response.data.data
					this.path_list=[{name:"root/",id:this.root_id}]
					console.log(this.path_list)
				}
			})
			// this.file_list = response.data.data
      // this.path_list=[{name:"root/",id:response.data.rootid}]
		})
	},
	methods: {
		forId: function (index) {
			return index
		},
		jumpHome: function (param1) {
			let number = param1
			let pid = this.project_list[number]['pid']
			Cookies.set('uid', this.uid)

			this.$router.push({ name: 'Home', query: { pid: pid } })
		},
    jumpPath: function (index) {
      console.log(index);
      axios.get('/api/filesystem/get_dir/?dir_id='+this.path_list[index].id, {}).then((response) => {
        console.log(response)
        this.file_list = response.data.data
        while(this.path_list.length!=index+1){
          this.path_list.pop();
        }        
      });  
    },
		jumpDir: function (id, name) {
			console.log(id, name)
      axios.get('/api/filesystem/get_dir/?dir_id='+id, {}).then((response) => {
        console.log(response)
        this.file_list = response.data.data
        this.path_list.push({name:name+'/',id:id});
      });      
		},
    jumpFile: function (id, name) {
			console.log(id, name)
      // axios.get('/api/filesystem/getdir/?pid='+id, {}).then((response) => {
      //   console.log(response)
      //   this.file_list = response.data.data
      //   this.path_list.push({name:name+'/',id:id});
      // });      
		},
    refreshPath: function (){
      this.jumpPath(this.path_list.length-1);
    },
    newDir: function (){
        for(let i = 0;i<this.file_list.length;++i)
        {
            let file=this.file_list[i];
            if(file.type=="1" && file.name==this.newDirForm.name)
            {
                ElMessage.warning("目录"+file.name+"已经存在")
                return
            }
        }
          
        let pattern = new RegExp('[\\\\/:*?\"<>|]');
        if(pattern.test(this.newDirForm.name)){
          ElMessage.error('目录不能包含【\\\\/:*?\"<>|】,请修改后重新上传!') 
          return;
        } 
        axios.post(`api/filesystem/create_dir/`,
          {pid:this.getPid(),dname:this.newDirForm.name},
        )
        .then((response) => {
					console.log(response)
            this.refreshPath()        
            this.newDirForm.visible = false
						this.newDirForm.name = '';
        })
        .catch(function (error) {
            console.log('error',error.response);
            if(error.response && error.response.status == 400)
                ElMessage.warning(error.response.data)
            else
                ElMessage.warning('未知错误')
        });
    },

		getAvatar: function (param1) {
			let src = '/static/avatar/' + param1 + '.jpg' + `?${Date.now()}`
			return src
		},
		getPid: function(){
			return this.path_list.at(-1).id;
		},
		beforeUpload: function (file) {
			const chunkSize = 4*1024*1024;
			const fileReader = new FileReader();
			const md5 = new SparkMD5();
			let index = 0;		
			file.md5=-1;	
			file.frag_md5_list = [];
			const loadFile = () => {
				const slice = file.slice(index, index + chunkSize);
				fileReader.readAsBinaryString(slice);
			}
			loadFile();
			// let axios=axios;
			// let pid=this.getPid();
			fileReader.onload = e => {
				file.frag_md5_list.push(SparkMD5.hashBinary(e.target.result))
				md5.appendBinary(e.target.result);
				if (index < file.size) {
					index += chunkSize;
					loadFile();
				} else {
					file.md5=md5.end();
				}
			};
			console.log(file)
			if(file.size>4*1024*1024)
				ElMessage.success("正在计算MD5码")

		},
    handleUploadSucess: () => {
      console.log("upload sucess")
    },
    handleFail: (error) => {
      ElMessage.error("无权限，文件上传失败!")
    },
		uploadFile: async function(request){
			function sleep(time) {
				return new Promise(resolve =>
				setTimeout(resolve,time)
				)}
			while(request.file.md5==-1){
				console.log("正在计算md5");
				await sleep(1);
			}
			if(request.file.size>4*1024*1024){
				ElMessage.success("MD5码计算完毕，开始上传")
			}
			console.log(request.file.md5,request.file.frag_md5_list);
			let parent_dir_id = this.getPid();
			axios.get('/api/file_exist',{params:{md5:request.file.md5}}).then((response)=>{
				console.log(response)
				if(response.data.data.is_exist){
					console.log("文件已存在")
					let content={ "filename": request.file.name, "md5": request.file.md5,"parent_dir_id": parent_dir_id }
					console.log(content)
					axios.post('/api/upload_file',content).then((response)=>{
						// console.log(response);
						if(response.data.message=="success"){
							ElMessage.success(`文件${request.file.name}秒传成功！`)
							this.refreshPath();
						}
						else{
							console.log("上传失败",response)
						}
					})
				}
				else{
					console.log("文件不存在")
					this.uploadFragments(request.file,parent_dir_id);

				}
			})
			// axios.post(`/api/transfer/uploadfile1/`,
      //   {pid:this.getPid(),fname:request.file.name,md5:request.file.md5}
      // )
			// .then((response)=>{
			// 	console.log(response);
			// 	if(response.status==201){
			// 		ElMessage.success(`正在上传${request.file.name}`);
			// 		let fd=new FormData();
			// 		fd.append('pid',this.getPid());
			// 		fd.append('md5',request.file.md5);
			// 		fd.append('file',request.file);
			// 		axios.post(`/api/transfer/uploadfile2/`,fd).then((response)=>{
			// 			console.log(response);
			// 		})
			// 		.then(()=>{
			// 			ElMessage.success(`上传${request.file.name}成功！`);
			// 			this.refreshPath();
			// 		})
			// 	}
			// 	else if(response.status==200){
			// 		ElMessage.success(`上传${request.file.name}成功！`);
			// 		this.refreshPath();
			// 	}
			// 	else{
			// 		ElMessage.error("上传失败")
			// 	}
			// })

		},
		uploadFragments: async function(file,parent_dir_id){
			const chunkSize = 4*1024*1024;
			let allocation_params={md5:file.md5,size:file.size}
			let response;
			let max_index = Math.ceil(file.size/(4*1024*1024));
			console.log(allocation_params);
			response= await axios.get('/api/upload_allocation',{params:allocation_params});
			console.log("upload allocation",response)
			let next_index=response.data.data.next_index;
			while(next_index!=-1){
				console.log("next index",next_index);
				response = await axios.get(`/api/file_fragment_exist?md5=${file.frag_md5_list[next_index]}`)
				console.log("fragment exist",response)
				let fragment_exist = response.data.data.is_exist;
				// let fragment_params = {index:next_index,file_md5:file.md5,fragment_md5:file.frag_md5_list[next_index]};
				let formData = new FormData()
				formData.append('index',next_index);
				formData.append('file_md5',file.md5);
				formData.append('fragment_md5',file.frag_md5_list[next_index])
				if(!fragment_exist){					
					formData.append('file_fragment',file.slice(next_index*chunkSize,next_index*chunkSize+chunkSize));
					console.log(formData);					
				}
				response= await axios.post('/api/upload_fragment',formData);
				console.log("upload fragment",response);

				ElMessage.success(`文件${file.name}上传进度:${Math.min((next_index+1)*4,file.size*1.0/1024/1024).toFixed(2)}MB/${Math.ceil(file.size*1.0/1024/1024).toFixed(2)}MB`)

				response= await axios.get('/api/upload_allocation',{params:allocation_params});
				console.log("upload allocation",response)
				next_index=response.data.data.next_index;
				// break;
			}			
			response = await axios.post('/api/upload_file',{filename:file.name,parent_dir_id:parent_dir_id,md5:file.md5});
			console.log("upload file",response);
			if(response.data.statusCode==200){
				ElMessage.success(`文件${file.name}上传成功！`)		
			}
			else{
				ElMessage.error(`文件${file.name}上传失败！`)		
			}
			this.refreshPath();	

		},
		handleExceed: function (files, fileList) {
    	ElMessage.warning(`当前限制选择 10 个文件，本次选择了 ${files.length} 个文件，共选择了 ${files.length + fileList.length} 个文件`);
    },
		rightClick: function (row, column, event) {
				// console.log(row, column, event)
				this.selectRow=row;
				event.preventDefault();
				this.removeMenu();
				let menu;
				if(row.type=='1'){
					menu=document.querySelector("#dirmenu");
					this.DirMenuVisible=true;
				}
				else if(row.type=='0'){
					menu=document.querySelector("#filemenu");
					this.FileMenuVisible=true;
				}
				this.styleMenu(event,menu);
    },
    removeMenu: function () {
        // 取消鼠标监听事件 菜单栏
        // console.log('removeTreeMenu')
        this.DirMenuVisible = false;
        this.FileMenuVisible = false;
        document.removeEventListener("click", this.removeMenu); // 关掉监听，
    },
    styleMenu: function (key, menu) {        
        document.addEventListener("click", this.removeMenu); // 给整个document新增监听鼠标事件，点击任何位置执行foo方法  
				console.log(menu)          
        menu.style.left = key.pageX  + "px";
        menu.style.top = key.pageY  + "px";
        //console.log('for menu style',menu.style.left,menu.style.top);
    },
		downloadFunc: async function () {
			console.log(this.selectRow);
			let row=this.selectRow;
			// this.browseFolder();
			let max_index = Math.ceil(row.fsize/(4*1024*1024));
			let index;
			let URL='/api/download_fragment';
			let response;
			let blobs = [];
			console.log(index,max_index);
			for(index=0;index<max_index;index++){
				let params={fdid:row.id,index};
				response = await axios.get(URL,{responseType: "blob",params});
				console.log(response)
				blobs.push(response.data);
				ElMessage.success(`文件${row.name}下载进度：${Math.min((index+1)*4,row.fsize/1024/1024).toFixed(2)}MB/${Math.ceil(row.fsize*1.0/1024/1024).toFixed(2)}MB`)
			}
			console.log(blobs)
			let finalBlob = new Blob(blobs);
			const filename = row.name;
			console.log(filename)
      let dom = document.createElement('a')
      let url = window.URL.createObjectURL(finalBlob)
      dom.href = url
      dom.download = decodeURI(filename)
        
      dom.style.display = 'none'
      document.body.appendChild(dom)
      dom.click()
      dom.parentNode.removeChild(dom)
      window.URL.revokeObjectURL(url)
      ElMessage.success(row.name+'下载成功!');  

		},
		
		deleteFunc: function () {
			console.log(this.selectRow);
			let row=this.selectRow;
			let URL=row.type=='0'?'/api/remove_file':'/api/remove_dir';
			let content=row.type=='0'?{fdid:row.id}:{dir_id:row.id}
			axios.delete(URL, {data:content}).then((response) => {
        console.log(response);
        ElMessage.success(row.name+'删除成功!');  
				this.refreshPath();
      });  
		},
		renameFunc: function () {
			console.log(this.selectRow);
			let row=this.selectRow;
			let URL=row.type=='0'?'/api/filesystem/rename_file/':'/api/filesystem/rename_dir/';
			let newName=this.renameForm.name;
			let content=row.type=='0'?{fid:row.id,fname:newName}:{did:row.id,dname:newName}
			if(newName==""){
				ElMessage.error("不能重命名为空！");
				return;
			}
			axios.put(URL, content).then((response) => {
        console.log(response);
        ElMessage.success(response.data.message);  
				this.renameForm.visible=false;
				this.renameForm.name='';
				this.refreshPath();
      })
			.catch((e)=>{
				console.log(e);
				ElMessage.error(newName+'已存在');  
			});  
		},
		openCopyForm: function(){
			axios.get("/api/share/get_dir_tree",{}).then((response)=>{
				console.log(response);
				this.fileTree=response.data.data;
				this.copyForm.visible=true;
			})
		},
		closeCopyForm: function(){
			this.copyForm.visible=false;			
			this.fileTree=[];
		},
		copyFunc: function (){
			let row=this.selectRow;
			let node=this.$refs.copyFileTree.getCurrentNode();
			let URL=row.type=='0'?'/api/share/copy_file/':'/api/share/copy_dir/';
			let content=row.type=='0'?{fdid:row.id,pid:node.did}:{did:row.id,pid:node.did};
			console.log(node);
			axios.post(URL, content).then((response) => {
        console.log(response);
				if(response.data.statusCode==200){
					ElMessage.success(row.name+'复制成功!');  
					this.refreshPath();
					this.closeCopyForm();
				}
				else{
					ElMessage.error(response.data.message);
				}
      })
			.catch((e)=>{
				ElMessage.error(e.response.data.data.detail)
			});	

		},
		openMoveForm: function(){
			axios.get("/api/share/get_dir_tree/",{}).then((response)=>{
				console.log(response);
				this.fileTree=response.data.data;
				console.log(this.fileTree)
				this.moveForm.visible=true;
			})
		},
		closeMoveForm: function(){
			this.moveForm.visible=false;			
			this.fileTree=[];
		},

		moveFunc: function (){
			let row=this.selectRow;
			let node=this.$refs.moveFileTree.getCurrentNode();
			let URL=row.type=='0'?'/api/share/move_file/':'/api/share/move_dir/';
			let content=row.type=='0'?{fdid:row.id,pid:node.did}:{did:row.id,pid:node.did};
			console.log(node);
			axios.post(URL, content).then((response) => {
        console.log(response);
				if(response.data.statusCode==200){
					ElMessage.success(row.name+'移动成功!');  
					this.refreshPath();
					this.closeMoveForm();
				}
				else{
					ElMessage.error(response.data.message);
				}
      })			
			.catch((e)=>{
				ElMessage.error(e.response.data.data.detail)
			});	
		},
		openShareForm: function(){
			let row=this.selectRow;
			axios.get("/api/share/getsharetoken/?type="+row.type+"&id="+row.id,{}).then((response)=>{
				
				this.shareForm.shareURL=window.location.origin+"/#/Share/"+response.data.token;
				this.shareForm.visible=true;

				console.log(this.shareForm.shareURL)
			})
		},
		closeShareForm: function(){
			this.shareForm.shareURL="";
			this.shareForm.visible=false;
		},
		copyShareURL: function(){
			this.$copyText(this.shareForm.shareURL)
      .then(() => {
        ElMessage.success('分享链接已复制到剪贴板！')
      })
      .catch(() => {
        ElMessage.error('分享链接复制到剪贴板失败！')
      })
		}




	},
}
</script>
 
<style scoped>
.box {
	display: flex;
	flex-direction: column;
	justify-content: space-between;
	align-items: center;
}
.el-collapse {
	margin-right: 50px;
	margin-left: 50px;
}
.el-descriptions {
	margin-top: 20px;
	margin-right: 70px;
	margin-left: 70px;
}
/deep/ .el-collapse-item__header {
	font-size: 20px;
}
.el-carousel__item h3 {
	display: flex;
	color: #475669;
	opacity: 0.75;
	line-height: 300px;
	margin: 0;
}

.el-carousel__item:nth-child(2n) {
	background-color: #99a9bf;
}

.el-carousel__item:nth-child(2n + 1) {
	background-color: #d3dce6;
}

.menu {
  position: absolute;
  background-color: #fff;
  width: 100px;
  /*height: 106px;*/
  font-size: 12px;
  color: #444040;
  border-radius: 4px;
  -webkit-box-sizing: border-box;
  box-sizing: border-box;
  border-radius: 3px;
  border: 1px solid rgba(0, 0, 0, 0.15);
  box-shadow: 0 6px 12px rgba(0, 0, 0, 0.175);
  white-space: nowrap;
  z-index: 1000;
}
.contextmenu_item {
  display: block;
  line-height: 34px;
  text-align: center;
}
.contextmenu_item:not(:last-child) {
  border-bottom: 1px solid rgba(0, 0, 0, 0.1);
}
.contextmenu_item:hover {
  cursor: pointer;
  background: #66b1ff;
  border-color: #66b1ff;
  color: #fff;
}

.el-container{

        height:100%;

}

.el-tree {
     display: inline-block;
      .el-tree-node {
        > .el-tree-node__children {
          overflow: unset;
        }
     }
}

.el-dialog-div{
    height: 50vh;
     overflow: auto;
    }
.el-dialog-div-share{
    height: 20vh;
     overflow: auto;
    }


</style>  