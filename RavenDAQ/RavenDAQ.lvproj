<?xml version='1.0' encoding='UTF-8'?>
<Project Type="Project" LVVersion="17008000">
	<Item Name="My Computer" Type="My Computer">
		<Property Name="server.app.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="server.control.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="server.tcp.enabled" Type="Bool">false</Property>
		<Property Name="server.tcp.port" Type="Int">0</Property>
		<Property Name="server.tcp.serviceName" Type="Str">My Computer/VI Server</Property>
		<Property Name="server.tcp.serviceName.default" Type="Str">My Computer/VI Server</Property>
		<Property Name="server.vi.callsEnabled" Type="Bool">true</Property>
		<Property Name="server.vi.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="specify.custom.address" Type="Bool">false</Property>
		<Item Name="RavenDAQ.vi" Type="VI" URL="../RavenDAQ.vi"/>
		<Item Name="Dependencies" Type="Dependencies">
			<Item Name="vi.lib" Type="Folder">
				<Item Name="FormatTime String.vi" Type="VI" URL="/&lt;vilib&gt;/express/express execution control/ElapsedTimeBlock.llb/FormatTime String.vi"/>
				<Item Name="NI_AALBase.lvlib" Type="Library" URL="/&lt;vilib&gt;/Analysis/NI_AALBase.lvlib"/>
				<Item Name="NI_AALPro.lvlib" Type="Library" URL="/&lt;vilib&gt;/Analysis/NI_AALPro.lvlib"/>
				<Item Name="subElapsedTime.vi" Type="VI" URL="/&lt;vilib&gt;/express/express execution control/ElapsedTimeBlock.llb/subElapsedTime.vi"/>
			</Item>
			<Item Name="lvanlys.so" Type="Document" URL="/&lt;resource&gt;/lvanlys.so"/>
			<Item Name="raven_decoder.vi" Type="VI" URL="../raven_decoder.vi"/>
		</Item>
		<Item Name="Build Specifications" Type="Build">
			<Item Name="RavenDAQ" Type="EXE">
				<Property Name="App_copyErrors" Type="Bool">true</Property>
				<Property Name="App_INI_aliasGUID" Type="Str">{68E1CA09-1160-11E8-91FF-0CC47A6A2280}</Property>
				<Property Name="App_INI_GUID" Type="Str">{68E1C8EC-1160-11E8-91FF-0CC47A6A2280}</Property>
				<Property Name="App_serverConfig.httpPort" Type="Int">8002</Property>
				<Property Name="Bld_autoIncrement" Type="Bool">true</Property>
				<Property Name="Bld_buildCacheID" Type="Str">{68E1E306-1160-11E8-91FF-0CC47A6A2280}</Property>
				<Property Name="Bld_buildSpecName" Type="Str">RavenDAQ</Property>
				<Property Name="Bld_excludeInlineSubVIs" Type="Bool">true</Property>
				<Property Name="Bld_excludeLibraryItems" Type="Bool">true</Property>
				<Property Name="Bld_excludePolymorphicVIs" Type="Bool">true</Property>
				<Property Name="Bld_localDestDir" Type="Path">../builds/NI_AB_PROJECTNAME/RavenDAQ</Property>
				<Property Name="Bld_localDestDirType" Type="Str">relativeToCommon</Property>
				<Property Name="Bld_modifyLibraryFile" Type="Bool">true</Property>
				<Property Name="Bld_previewCacheID" Type="Str">{68E1E295-1160-11E8-91FF-0CC47A6A2280}</Property>
				<Property Name="Bld_version.build" Type="Int">1</Property>
				<Property Name="Bld_version.major" Type="Int">1</Property>
				<Property Name="DestinationCount" Type="Int">2</Property>
				<Property Name="Destination[0].destName" Type="Str">Application</Property>
				<Property Name="Destination[0].path" Type="Path">../builds/NI_AB_PROJECTNAME/RavenDAQ/Application</Property>
				<Property Name="Destination[0].preserveHierarchy" Type="Bool">true</Property>
				<Property Name="Destination[0].type" Type="Str">App</Property>
				<Property Name="Destination[1].destName" Type="Str">Support Directory</Property>
				<Property Name="Destination[1].path" Type="Path">../builds/NI_AB_PROJECTNAME/RavenDAQ/data</Property>
				<Property Name="SourceCount" Type="Int">2</Property>
				<Property Name="Source[0].itemID" Type="Str">{66D6283B-1160-11E8-A973-0CC47A6A2280}</Property>
				<Property Name="Source[0].type" Type="Str">Container</Property>
				<Property Name="Source[1].destinationIndex" Type="Int">0</Property>
				<Property Name="Source[1].itemID" Type="Ref">/My Computer/RavenDAQ.vi</Property>
				<Property Name="Source[1].sourceInclusion" Type="Str">TopLevel</Property>
				<Property Name="Source[1].type" Type="Str">VI</Property>
				<Property Name="TgtF_fileDescription" Type="Str">RavenDAQ</Property>
				<Property Name="TgtF_internalName" Type="Str">RavenDAQ</Property>
				<Property Name="TgtF_legalCopyright" Type="Str">Copyright 2018 </Property>
				<Property Name="TgtF_productName" Type="Str">RavenDAQ</Property>
				<Property Name="TgtF_targetfileGUID" Type="Str">{68E1E134-1160-11E8-91FF-0CC47A6A2280}</Property>
				<Property Name="TgtF_targetfileName" Type="Str">Application</Property>
				<Property Name="TgtF_versionIndependent" Type="Bool">true</Property>
			</Item>
		</Item>
	</Item>
</Project>
