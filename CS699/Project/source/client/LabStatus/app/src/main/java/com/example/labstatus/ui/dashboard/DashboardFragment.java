package com.example.labstatus.ui.dashboard;

import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.ColorStateList;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.fragment.app.Fragment;

import androidx.lifecycle.ViewModelProviders;
import androidx.recyclerview.widget.GridLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.concurrent.TimeUnit;

import android.widget.ProgressBar;


import com.example.labstatus.DetailActivity;
import com.example.labstatus.MainActivity;
import com.example.labstatus.R;


import com.google.android.material.bottomnavigation.BottomNavigationView;
import com.google.android.material.floatingactionbutton.FloatingActionButton;
import com.google.android.material.snackbar.Snackbar;


import org.json.JSONException;
import org.json.JSONObject;

import okhttp3.Call;
import okhttp3.Callback;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;


public class DashboardFragment extends Fragment implements RecyclerViewAdapterSL1.ItemListener {

    private DashboardViewModel dashboardViewModel;
    RecyclerView recyclerView;
    ArrayList arrayList;
    RecyclerViewAdapterSL1 adapter;
    int keyboard_request=0;
    int shutdown_request=0;
    int mouse_request=0;
    int lab_status=0;
    public String TAG = DashboardFragment.class.getSimpleName();
    HashMap<String, Boolean> status=new HashMap<>();
    String SL1_ini="10.130.153.";
    String ServerIP="10.130.150.223";
    int port=5000;
    String lab="sl1";

    BottomNavigationView navView;

    String url = "http://"+ServerIP+":"+port +"/lab/";
    ProgressBar bar;
    View root;
    int ShutDownFlag=1;
    AlertDialog.Builder builder;
    boolean isFABOpen=false;
    FloatingActionButton fab_lab,fab_keyboard,fab_mouse,fab,fab_wakeup,fab_refresh;


    public View onCreateView(@NonNull LayoutInflater inflater,
                             ViewGroup container, Bundle savedInstanceState) {
        dashboardViewModel =
                ViewModelProviders.of(this).get(DashboardViewModel.class);
        root = inflater.inflate(R.layout.fragment_dashboard, container, false);

        builder = new AlertDialog.Builder(getActivity());
        bar=root.findViewById(R.id.pBar_SL1);
        bar.setVisibility(root.GONE);
    /*    final TextView textView = root.findViewById(R.id.text_dashboard);
        dashboardViewModel.getText().observe(this, new Observer<String>() {
            @Override
            public void onChanged(@Nullable String s) {
                textView.setText(s);
            }
        });*/

        recyclerView = root.findViewById(R.id.recyclerViewSL1);
        arrayList = new ArrayList();
        fab =  root.findViewById(R.id.fab_sl1);
        fab_lab =  root.findViewById(R.id.fab_lab_sl1);
        fab_keyboard =  root.findViewById(R.id.fab_keyboard_sl1);
        fab_mouse =  root.findViewById(R.id.fab_mouse_sl1);
        fab_wakeup=root.findViewById(R.id.fab_wakeup_sl1);
        fab_refresh=root.findViewById(R.id.fab_sl1_refresh);
        navView=((MainActivity)this.getActivity()).getNavView();
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if(!isFABOpen){
                    showFABMenu();
                }else{
                    closeFABMenu();
                }
            }
        });
        fab_refresh.setOnClickListener(new View.OnClickListener(){

            @Override
            public void onClick(View v) {
                if (lab_status == 0)
                {
                    lab_status=1;
                    fab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                    fab.setImageResource(R.drawable.ic_refresh);
                    fab_wakeup.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));
                    fab_refresh.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                    fab_keyboard.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));
                    fab_wakeup.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));
                    fab_lab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));
                    fab_mouse.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));
                    try {
                        GetLabStatus();
                    } catch (Exception e) {
                        Log.e(TAG, e.toString());
                    }
                }
                else{
                    showSnackBarMessage("Please wait for previous request completion");
                }
            }
        });

        fab_lab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (shutdown_request == 0)
                {
                    shutdown_request=1;
                    fab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                    fab.setImageResource(R.drawable.ic_lab);
                    fab_wakeup.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));


                    fab_keyboard.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));
                    fab_mouse.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));


                    builder.setMessage("Do you want to shut down all pc of SL1 Lab?")
                            .setCancelable(false)
                            .setPositiveButton("Yes", new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int id) {
                                    fab_lab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.color_hardware)));
                                    ShutDownLab();
                                    //new ShutDownLab().execute();
                                }
                            })
                            .setNegativeButton("No", new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int id) {
                                    fab_lab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));
                                    dialog.cancel();

                                }
                            });

                    AlertDialog alert = builder.create();
                    alert.setTitle("Shut down");
                    alert.show();
                }
                else{
                    showSnackBarMessage("Please wait for previous request completion");
                }



            }
        });
        fab_wakeup.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                fab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                fab.setImageResource(R.drawable.ic_wakeup);
                fab_wakeup.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));

                fab_keyboard.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));
                fab_wakeup.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                fab_lab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));
                fab_mouse.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));
                WakeOnLAN();
                // new WakeOnLAN().execute();

            }
        });
        fab_keyboard.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (keyboard_request == 0){
                    keyboard_request=1;
                    fab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                    fab.setImageResource(R.drawable.ic_keyboard);
                    fab_wakeup.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));

                    fab_keyboard.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                    fab_lab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));
                    fab_mouse.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));
                    GetKeyboardStatus();
                    //new GetKeyboardStatus().execute();
                }
                else{
                    showSnackBarMessage("Please wait for previous request completion");
                }
            }
        });
        fab_mouse.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mouse_request == 0)
                {
                    mouse_request=1;
                    fab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                    fab.setImageResource(R.drawable.ic_mouse);
                    fab_wakeup.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));

                    fab_mouse.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                    fab_keyboard.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));
                    fab_lab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));
                    GetMouseStatus();
                    //new GetMouseStatus().execute();
                }
                else{
                    showSnackBarMessage("Please wait for previous request completion");
                }
            }
        });


        arrayList = new ArrayList();
        //new GetLabStatus().execute();
        try {
            GetLabStatus();
        }
        catch (Exception e){
            Log.e(TAG,e.toString());
        }

        return root;
    }





    private void showFABMenu(){
        isFABOpen=true;
        fab.setImageResource(R.drawable.ic_fab_right);
        fab_wakeup.animate().translationX(-getResources().getDimension(R.dimen.standard_65));

        fab_lab.animate().translationX(-getResources().getDimension(R.dimen.standard_115));

        fab_keyboard.animate().translationX(-getResources().getDimension(R.dimen.standard_165));
        fab_mouse.animate().translationX(-getResources().getDimension(R.dimen.standard_215));
        fab_refresh.animate().translationX(-getResources().getDimension(R.dimen.standard_265));
    }

    private void closeFABMenu(){
        isFABOpen=false;
        // fab.setImageResource(R.drawable.ic_fab_left);
        fab_lab.animate().translationX(0);
        fab_wakeup.animate().translationX(0);
        fab_keyboard.animate().translationX(0);
        fab_mouse.animate().translationX(0);
        fab_refresh.animate().translationX(0);
    }
  /*  private void ena_dis_UI_elements(boolean en_dis){
        adapter.isClickable=en_dis;
        try {

            for (int i = 0; i < navView.getMenu().size(); i++) {
                navView.getMenu().getItem(i).setEnabled(en_dis);
            }
        }
        catch(Exception e){
            Log.e(TAG,e.toString());
        }

    }*/

    private void GetLabStatus() throws IOException
    {

        preprocessing_task();
        OkHttpClient client = new OkHttpClient.Builder()
                .connectTimeout(300, TimeUnit.SECONDS)
                .writeTimeout(300,TimeUnit.SECONDS)
                .readTimeout(300,TimeUnit.SECONDS)
                .build();
        String urltemp=url;
        urltemp+=lab+"/status";
        Log.e(TAG,"Sending request :"+urltemp);
        //  String url = "http://10.130.150.223:5000/lab/SL1/status";
        Request request = new Request.Builder()
                .url(urltemp)
                .build();

        client.newCall(request).enqueue(new Callback() {
            @Override
            public void onFailure(Call call, IOException e) {
                call.cancel();
                Log.e(TAG,e.toString());
            }

            @Override
            public void onResponse(Call call, final Response response) throws IOException {
                try{
                int httpResponseCode = response.code();
                Log.e(TAG, "resp code=" + httpResponseCode);
                if (httpResponseCode == 200) {
                    final String myResponse = response.body().string();

                    getActivity().runOnUiThread(new Runnable() {
                        @Override
                        public void run() {

                            get_info(myResponse);
                            update_UI("status");
                            post_processing_task();
                            lab_status = 0;


                        }
                    });
                } else {
                    showSnackBarMessage("Could not connect with server");
                    post_processing_task();
                }
            }catch (Exception e) {
                Log.e(TAG, e.toString());
            }

            }
        });
    }

    private void showSnackBarMessage(String msg) {
        try {
            Log.e(TAG, msg);
            final String temp = msg;
            getActivity().runOnUiThread(new Runnable() {
                @Override
                public void run() {

                    Snackbar.make(root, temp, Snackbar.LENGTH_LONG)
                            .setAction("Action", null).show();
                }
            });
        }
        catch(Exception e){
            Log.e(TAG,e.toString());
        }
    }

    private void preprocessing_task() {
        try {
            Log.e(TAG, "Performing preprocessing task");
            getActivity().runOnUiThread(new Runnable() {
                @Override
                public void run() {

                    bar.setVisibility(root.VISIBLE);
                }
            });
            arrayList.clear();
            status.clear();
        }
        catch(Exception e){
            Log.e(TAG,e.toString());
        }
    }

    private void update_UI(String operation) {
        try {
            String color="#008577";
            String error_color="#D81B60";

            if(operation=="keyboard" || operation == "mouse"){

                error_color ="#927B01";
            }

            String label;
            Integer no;
            for (int i = -1; i < 9; i++) {
                for (int j = 1; j <= 13; j++) {

                    if (i == -1) {
                        if (j == 6) {
                            no = 200;
                            label = no.toString();
                        } else {
                            label = "";
                        }
                    } else {
                        if (j == 6) {
                            label = "";
                        } else {
                            if (j > 6) {
                                no = ((7 * i) + j) - 6;
                                if (no > 83) {
                                    label = "";
                                } else {
                                    label = no.toString();
                                }
                            } else {
                                no = ((5 * i) + j) + 63;
                                if (no > 83) {
                                    label = "";
                                } else {
                                    label = no.toString();
                                }
                            }
                        }
                    }

                    if (label != "") {
                        String ip = SL1_ini + label;
                        boolean temp = false;
                        if (status.get(ip) != null) {
                            temp = status.get(ip);
                        }

                        if (temp == true) {
                            arrayList.add(new DashboardViewModel(label, color));
                        } else {
                            arrayList.add(new DashboardViewModel(label, error_color));
                        }
                    } else {
                        arrayList.add(new DashboardViewModel(label, error_color));
                    }
                    // arrayList.add(new DashboardViewModel(label, "#FF0000"));
                }
            }
            recyclerView.invalidate();
            adapter = new RecyclerViewAdapterSL1(getContext(), arrayList, DashboardFragment.this);
            recyclerView.setAdapter(adapter);


            /**
             AutoFitGridLayoutManager that auto fits the cells by the column width defined.
             **/

        /*AutoFitGridLayoutManager layoutManager = new AutoFitGridLayoutManager(this, 500);
        recyclerView.setLayoutManager(layoutManager);*/


            /**
             Simple GridLayoutManager that spans two columns
             **/
            GridLayoutManager manager = new GridLayoutManager(getContext(), 13, GridLayoutManager.VERTICAL, false);
            recyclerView.setLayoutManager(manager);


        }
        catch(Exception e){
            Log.e(TAG,e.getMessage());
        }
    }


    private void post_processing_task(){
        try {
            getActivity().runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    bar.setVisibility(root.GONE);
                }
            });
        }
        catch(Exception e){
            Log.e(TAG,e.toString());
        }
    }

    private void get_info(String response) {
        arrayList.clear();
        status.clear();
        if (response != null) {
            try {
                JSONObject jsonObj = new JSONObject(response);


                Iterator<String> iter = jsonObj.keys();
                while (iter.hasNext()) {
                    String key = iter.next();
                    try {
                        Boolean value = (Boolean) jsonObj.get(key);
                        status.put(key,value);
                    } catch (JSONException e) {
                        // Something went wrong!
                    }
                }



            } catch (final JSONException e) {
                Log.e(TAG, "Json parsing error: " + e.getMessage());

            }

        } else {

            showSnackBarMessage("Could not connect with server");
            post_processing_task();
        }
    }

    private void GetKeyboardStatus(){
        //preprocessing_task();

      //  ena_dis_UI_elements(false);
        preprocessing_task();
        OkHttpClient client = new OkHttpClient.Builder()
                .connectTimeout(300,TimeUnit.SECONDS)
                .writeTimeout(300,TimeUnit.SECONDS)
                .readTimeout(300,TimeUnit.SECONDS)
                .build();

        String urltemp=url;
        urltemp+=lab+"/hardware/keyboard";
        Log.e(TAG,"Sending request :"+urltemp);
        //  String url = "http://10.130.150.223:5000/lab/SL1/status";
        Request request = new Request.Builder()
                .url(urltemp)
                .build();
        showSnackBarMessage("Request Send.Please wait ...");
        client.newCall(request).enqueue(new Callback() {
            @Override
            public void onFailure(Call call, IOException e) {
        try{
                call.cancel();
                showSnackBarMessage("Could not connect with server");
                post_processing_task();

            }catch (Exception e1) {
                Log.e(TAG, e.toString());
            }
               // ena_dis_UI_elements(true);
                keyboard_request=0;
            }

            @Override
            public void onResponse(Call call, final Response response) throws IOException {
               try{ int httpResponseCode = response.code();
                if (httpResponseCode == 200) {
                    final String myResponse = response.body().string();

                    getActivity().runOnUiThread(new Runnable() {
                        @Override
                        public void run() {

                            get_info(myResponse);
                            update_UI("keyboard");
                            //post_processing_task();
                            //ena_dis_UI_elements(true);
                            keyboard_request = 0;
                            closeFABMenu();
                            fab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                            fab.setImageResource(R.drawable.ic_keyboard);

                        }
                    });
                } else {
                    showSnackBarMessage("Could not connect with server");
                    post_processing_task();
                   // ena_dis_UI_elements(true);
                    keyboard_request = 0;
                }
                post_processing_task();

            }catch (Exception e) {
                Log.e(TAG, e.toString());
            }
            }
        });
    }
    private void GetMouseStatus(){
        //preprocessing_task();
        preprocessing_task();
        //ena_dis_UI_elements(false);
        OkHttpClient client = new OkHttpClient.Builder()
                .connectTimeout(300,TimeUnit.SECONDS)
                .writeTimeout(300,TimeUnit.SECONDS)
                .readTimeout(300,TimeUnit.SECONDS)
                .build();

        String urltemp=url;
        urltemp+=lab+"/hardware/mouse";
        Log.e(TAG,"Sending request :"+urltemp);
        //  String url = "http://10.130.150.223:5000/lab/SL1/status";
        Request request = new Request.Builder()
                .url(urltemp)
                .build();
        showSnackBarMessage("Request Send.Please wait ...");
        client.newCall(request).enqueue(new Callback() {
            @Override
            public void onFailure(Call call, IOException e) {
                try{
                call.cancel();
                showSnackBarMessage("Could not connect with server");
                post_processing_task();
              //  ena_dis_UI_elements(true);
                mouse_request=0;}
                catch (Exception e1) {
                    Log.e(TAG, e.toString());
                }
            }

            @Override
            public void onResponse(Call call, final Response response) throws IOException {
             try{   int httpResponseCode = response.code();
                if (httpResponseCode == 200) {
                    final String myResponse = response.body().string();

                    getActivity().runOnUiThread(new Runnable() {
                        @Override
                        public void run() {

                            get_info(myResponse);
                            update_UI("mouse");
                            //post_processing_task();
                            //ena_dis_UI_elements(true);
                            mouse_request = 0;
                            closeFABMenu();
                            fab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                            fab.setImageResource(R.drawable.ic_mouse);

                        }
                    });
                } else {
                    showSnackBarMessage("Could not connect with server");
                    post_processing_task();
                    //ena_dis_UI_elements(true);
                    mouse_request = 0;
                }
                post_processing_task();
            }catch (Exception e) {
                Log.e(TAG, e.toString());
            }
            }
        });
    }
    private void WakeOnLAN(){
        preprocessing_task();

        OkHttpClient client = new OkHttpClient.Builder()
                .connectTimeout(300,TimeUnit.SECONDS)
                .writeTimeout(300,TimeUnit.SECONDS)
                .readTimeout(300,TimeUnit.SECONDS)
                .build();

        String urltemp=url;
        urltemp+=lab+"/wakeup";
        Log.e(TAG,"Sending request :"+urltemp);
        //  String url = "http://10.130.150.223:5000/lab/SL1/status";
        Request request = new Request.Builder()
                .url(urltemp)
                .build();
        showSnackBarMessage("Sending WakeOnLan Request to SL1 Lab");
        client.newCall(request).enqueue(new Callback() {
            @Override
            public void onFailure(Call call, IOException e) {
                call.cancel();
            }

            @Override
            public void onResponse(Call call, final Response response) throws IOException {
                try {
                    int httpResponseCode = response.code();
                    if (httpResponseCode == 200) {
                        final String myResponse = response.body().string();

                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                showSnackBarMessage("WakeOnLAN request to SL1 Lab complete");
                                post_processing_task();

                            }
                        });
                    } else {
                        showSnackBarMessage("Could not Connect with server");
                    }
                    post_processing_task();
                }
                catch(Exception e1){
                    Log.e(TAG,e1.toString());
                }
            }
        });
    }

    private void ShutDownLab(){
        //ena_dis_UI_elements(false);
        preprocessing_task();
        OkHttpClient client = new OkHttpClient.Builder()
                .connectTimeout(300,TimeUnit.SECONDS)
                .writeTimeout(300,TimeUnit.SECONDS)
                .readTimeout(300,TimeUnit.SECONDS)
                .build();

        String urltemp=url;
        urltemp+=lab+"/shutdown";
        Log.e(TAG,"Sending request :"+urltemp);
        //  String url = "http://10.130.150.223:5000/lab/SL1/status";
        Request request = new Request.Builder()
                .url(urltemp)
                .build();
        showSnackBarMessage("Sending ShutDown Request to SL1 Lab");
        client.newCall(request).enqueue(new Callback() {
            @Override
            public void onFailure(Call call, IOException e) {
                try{
                call.cancel();
                showSnackBarMessage("Could not connect with server");
                //ena_dis_UI_elements(true);
                shutdown_request = 0;
                post_processing_task();
            }catch (Exception e1     ) {
                    Log.e(TAG, e.toString());
                }
            }

            @Override
            public void onResponse(Call call, final Response response) throws IOException {
               try{
                int httpResponseCode = response.code();
                if (httpResponseCode == 200) {
                    final String myResponse = response.body().string();

                    getActivity().runOnUiThread(new Runnable() {
                        @Override
                        public void run() {

                            showSnackBarMessage("ShutDown Request to SL1 Lab complete");
                            closeFABMenu();
                            fab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                            fab.setImageResource(R.drawable.ic_lab);

                        }
                    });
                } else {
                    showSnackBarMessage("Could not Connect with server");
                }
                shutdown_request = 0;
                //  ena_dis_UI_elements(true);
                   post_processing_task();
            }catch (Exception e) {
                Log.e(TAG, e.toString());
            }
            }
        });
    }



    @Override
    public void onItemClick(DashboardViewModel item) {
        try {
            //Toast.makeText(getContext(), item.text + " is clicked", Toast.LENGTH_SHORT).show();
            Intent intent = new Intent(getActivity(), DetailActivity.class);
            intent.putExtra("lab", "sl1");
            intent.putExtra("pc", item.text);

            if (status.get(SL1_ini + item.text) == false)
                Snackbar.make(root, "System offline", Snackbar.LENGTH_LONG)
                        .setAction("Action", null).show();
            else {
                startActivity(intent);
            }
        }
        catch(Exception e){
            Log.e(TAG,e.toString());
        }
    }



    private void ExamModeOn(){
        preprocessing_task();
        //ena_dis_UI_elements(false);
        OkHttpClient client = new OkHttpClient.Builder()
                .connectTimeout(300,TimeUnit.SECONDS)
                .writeTimeout(300,TimeUnit.SECONDS)
                .readTimeout(300,TimeUnit.SECONDS)
                .build();

        String urltemp=url;
        urltemp+=lab+"/exam_mode_on";
        Log.e(TAG,"Sending request :"+urltemp);
        //  String url = "http://10.130.150.223:5000/lab/sl2/status";
        Request request = new Request.Builder()
                .url(urltemp)
                .build();
        showSnackBarMessage("Sending Exam Mode Request to SL2 Lab");
        client.newCall(request).enqueue(new Callback() {
            @Override
            public void onFailure(Call call, IOException e) {
                try {
                    call.cancel();
                    showSnackBarMessage("Could not connect with server");
                    //ena_dis_UI_elements(true);
                    shutdown_request = 0;
                    post_processing_task();
                }
                catch(Exception e1){
                    Log.e(TAG,e1.toString());
                }
            }

            @Override
            public void onResponse(Call call, final Response response) throws IOException {
                try{
                    int httpResponseCode=response.code();
                    if (httpResponseCode == 200) {
                        final String myResponse = response.body().string();

                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {

                                showSnackBarMessage("Exam Mode setup in SL2 Lab completed");
                                closeFABMenu();
                                fab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                                fab.setImageResource(R.drawable.ic_lab);

                            }
                        });
                    }
                    else{
                        showSnackBarMessage("Could not Connect with server");
                    }
                    shutdown_request=0;
                    post_processing_task();
                    //  ena_dis_UI_elements(true);
                }
                catch(Exception e){
                    Log.e(TAG,e.toString());
                }
            }
        });
    }

    private void ExamModeOff(){
        preprocessing_task();
        //ena_dis_UI_elements(false);
        OkHttpClient client = new OkHttpClient.Builder()
                .connectTimeout(300,TimeUnit.SECONDS)
                .writeTimeout(300,TimeUnit.SECONDS)
                .readTimeout(300,TimeUnit.SECONDS)
                .build();

        String urltemp=url;
        urltemp+=lab+"/exam_mode_off";
        Log.e(TAG,"Sending request :"+urltemp);
        //  String url = "http://10.130.150.223:5000/lab/sl2/status";
        Request request = new Request.Builder()
                .url(urltemp)
                .build();
        showSnackBarMessage("Disabling Exam Mode in SL1 Lab");
        client.newCall(request).enqueue(new Callback() {
            @Override
            public void onFailure(Call call, IOException e) {
                try {
                    call.cancel();
                    showSnackBarMessage("Could not connect with server");
                    //ena_dis_UI_elements(true);
                    shutdown_request = 0;
                    post_processing_task();
                }
                catch(Exception e1){
                    Log.e(TAG,e1.toString());
                }
            }

            @Override
            public void onResponse(Call call, final Response response) throws IOException {
                try{
                    int httpResponseCode=response.code();
                    if (httpResponseCode == 200) {
                        final String myResponse = response.body().string();

                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {

                                showSnackBarMessage("Exam Mode disabled in SL1 Lab");
                                closeFABMenu();
                                fab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                                fab.setImageResource(R.drawable.ic_lab);

                            }
                        });
                    }
                    else{
                        showSnackBarMessage("Could not Connect with server");
                    }
                    shutdown_request=0;
                    post_processing_task();
                    //  ena_dis_UI_elements(true);
                }
                catch(Exception e){
                    Log.e(TAG,e.toString());
                }
            }
        });
    }


    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        // Do something that differs the Activity's menu here
        super.onCreateOptionsMenu(menu, inflater);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {

            case R.id.exam_mode_on:
                ExamModeOn();

                break;
            case R.id.exam_mode_off:
                ExamModeOff();
                break;

            default:
                break;
        }

        return false;
    }



}