package com.example.labstatus;

import android.content.res.ColorStateList;
import android.os.AsyncTask;
import android.os.Bundle;

import com.google.android.material.floatingactionbutton.FloatingActionButton;
import com.google.android.material.snackbar.Snackbar;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import androidx.recyclerview.widget.DividerItemDecoration;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import android.os.Handler;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.View;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.TextView;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
/** @brief Shows details like CPU stats's, list of intalled process, hardware device connected etc

 @author Area51
 @date Nov 2019
 */
public class DetailActivity extends AppCompatActivity {
    String cpu="CPU Utilization :";
    String core=""; //"Core 1: 100%\t\t\t\t\t\t\tCore 2: 100%\nCore 3: 100%\\t\\t\\t\\t\\t\\t\\tCore 4: 100%";
    String lab,pc;
    ProgressBar bar;
    String TAG="Datail Activity";

    TextView cpu_tv,core_tv;
    int counter=0;
    FloatingActionButton fab_keyboard,fab_mouse,fab;
    boolean isFABOpen=false;
    Boolean isKeyboardConnected=false;
    Boolean isMouseConnected=false;
    View root;
    RecyclerView recyclerView;
    EditText editText;
    SoftListAdapter mAdapter;
     RecyclerView.LayoutManager layoutManager;
    ArrayList<String> software=new ArrayList<>();

    final Handler handler = new Handler();
    HashMap<String,Double> status=new HashMap<>();
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_detail);
        lab=getIntent().getStringExtra("lab");
        pc=getIntent().getStringExtra("pc");
        root=findViewById(R.id.corLay);
        bar=findViewById(R.id.pBar_detail);
        bar.setVisibility(View.GONE);
        editText=findViewById(R.id.soft_search);
        recyclerView=findViewById(R.id.my_recycler_view_lst);
        layoutManager = new LinearLayoutManager(this);
        recyclerView.setLayoutManager(layoutManager);
        recyclerView.setHasFixedSize(true);
        DividerItemDecoration dividerItemDecoration = new DividerItemDecoration(recyclerView.getContext(),DividerItemDecoration.VERTICAL);
        recyclerView.addItemDecoration(dividerItemDecoration);
        // specify an adapter (see also next example)

        cpu_tv=findViewById(R.id.cpu_utilization);
        core_tv=findViewById(R.id.core);
        String lab_pc=lab+"-"+pc;
        setTitle(lab_pc);

        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);


        fab =  findViewById(R.id.fab_detail);
    //    fab_lab =  findViewById(R.id.fab_lab_detail);
        fab_keyboard =  findViewById(R.id.fab_keyboard_detail);
        fab_mouse =  findViewById(R.id.fab_mouse_detail);
       // fab_search =  findViewById(R.id.fab_search_detail);
        editText.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence charSequence, int i, int i1, int i2) {
                closeFABMenu();
            }

            @Override
            public void onTextChanged(CharSequence charSequence, int i, int i1, int i2) {

            }

            @Override
            public void afterTextChanged(Editable editable) {
                //after the change calling the method and passing the search input
                filter(editable.toString());
            }
        });
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
              /*  Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG)
                        .setAction("Action", null).show();*/
                if(!isFABOpen){
                    showFABMenu();
                }else{
                    closeFABMenu();
                }
            }
        });



            handler.post(new Runnable() {
                @Override
                public void run() {
                    //Do something after 20 seconds
                    new GetDetails().execute();
                    handler.postDelayed(this, 1000);
                }
            });
            new GetHardwareDetails().execute();
            new GetSoftwares().execute();




    }
    /**
     * Expands the floating actions button which are stacked over one another.
     */
    private void showFABMenu(){
        isFABOpen=true;
        fab.setImageResource(R.drawable.ic_down);
       // fab_lab.animate().translationY(-getResources().getDimension(R.dimen.standard_65));
       // fab_search.animate().translationY(-getResources().getDimension(R.dimen.standard_115));
        fab_keyboard.animate().translationY(-getResources().getDimension(R.dimen.standard_65));
        fab_mouse.animate().translationY(-getResources().getDimension(R.dimen.standard_115));
    }
    /**
     * Closes the expanded floating actions button.
     */

    private void closeFABMenu(){
        isFABOpen=false;
        fab.setImageResource(R.drawable.ic_up);
      //  fab_lab.animate().translationY(0);
      //  fab_search.animate().translationY(0);
        fab_keyboard.animate().translationY(0);
        fab_mouse.animate().translationY(0);
    }

    /**
     * Filters the list of softwares according to given text.
     * @param text text for filtering list
     */
    private void filter(String text) {
        //new array list that will hold the filtered data
        ArrayList<String> filterdNames = new ArrayList<>();

        //looping through existing elements
        for (String s : software) {
            //if the existing elements contains the search input
            if (s.toLowerCase().contains(text.toLowerCase())) {
                //adding the element to filtered list
                filterdNames.add(s);
            }
        }

        //calling a method of the adapter class and passing the filtered list
        mAdapter.filterList(filterdNames);
    }

    /**
     * Async task to get the details of CPU and its core utilization
     * @see android.os.AsyncTask
     *
     */
    private class GetDetails extends AsyncTask<Void, Void, Void> {


        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            if(counter==0) {


                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        bar.setVisibility(View.VISIBLE);
                    }
                });
            }

        }

        @Override
        protected Void doInBackground(Void... arg0) {
            HttpHandler sh = new HttpHandler();
            // Making a request to url and getting response
            String url = "http://10.130.150.223:5000/lab/"+lab+"/"+pc+"/cpu_stat";
            String jsonStr = sh.makeServiceCall(url,true);


            //Log.e(TAG, "Response from url: " + jsonStr);
            if (jsonStr != null) {
                try {
                    JSONObject jsonObj = new JSONObject(jsonStr);

                    Iterator<String> iter = jsonObj.keys();
                    while (iter.hasNext()) {
                        String key = iter.next();
                        try {
                            Double value = (Double) jsonObj.get(key);
                            status.put(key,value);
                        } catch (JSONException e) {
                            // Something went wrong!
                        }
                    }


                } catch (final JSONException e) {
                    Log.e(TAG, "Json parsing error: " + e.getMessage());

                }

            } else {
                Log.e(TAG, "Couldn't get json from server.");
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {

                        Snackbar.make(root, "Could not connect with server", Snackbar.LENGTH_LONG)
                                .setAction("Action", null).show();
                    }
                });


            }

            return null;
        }

        @Override
        protected void onPostExecute(Void result) {
            super.onPostExecute(result);
            /*ListAdapter adapter = new SimpleAdapter(MainActivity.this, contactList,
                    R.layout.list_item, new String[]{ "email","mobile"},
                    new int[]{R.id.email, R.id.mobile});
            lv.setAdapter(adapter);*/
            try {
                String label;
                Integer no;
                Double util;
                String temp;
                Double temp_cpu_util=status.get("cpu");
                String temp_cpu_String=String.format("%3.2f",temp_cpu_util) + "%";
                cpu+=temp_cpu_String;
                for(int i=1;i<status.size();i++){
                   if(i%2==0)
                   {
                       String temp1="cpu"+(i-1);
                       util=status.get(temp1);
                       temp=String.format("Core %d: %.2f\n",i,util);
                       core+=temp;

                   }else{
                       String temp1="cpu"+(i-1);
                       util=status.get(temp1);
                       temp=String.format("Core %d: %-20.2f",i,util);
                       core+=temp;
                   }

                }
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        cpu_tv.setText(cpu);
                        core_tv.setText(core);
                       if(counter==0) {

                            counter=1;

                           bar.setVisibility(View.GONE);
                       }

                    }
                });
                 cpu="CPU Utilization :";
                 core="";
            }
            catch(Exception e){
                Log.e(TAG,e.getMessage());
            }
        }
    }


    @Override
    public void onBackPressed() {
        super.onBackPressed();
        handler.removeCallbacksAndMessages(null);
    }
    /**
     * Gets the details of hardware devices such as keyboard and mouse.
     * @see android.os.AsyncTask
     */
    private class GetHardwareDetails extends AsyncTask<Void, Void, Void> {


        @Override
        protected void onPreExecute() {
            super.onPreExecute();


        }

        @Override
        protected Void doInBackground(Void... arg0) {
            HttpHandler sh = new HttpHandler();
            // Making a request to url and getting response
            String url = "http://10.130.150.223:5000/lab/"+lab+"/"+pc+"/hardware";
            String jsonStr = sh.makeServiceCall(url,true);


            //Log.e(TAG, "Response from url: " + jsonStr);
            if (jsonStr != null) {
                try {

                    JSONObject jsonObj = new JSONObject(jsonStr);
                    isKeyboardConnected=(Boolean)jsonObj.get("keyboard");
                    isMouseConnected=(Boolean)jsonObj.get("mouse");
/*                    while (iter.hasNext()) {
                        String key = iter.next();
                        try {
                            Double value = (Double) jsonObj.get(key);
                            status.put(key,value);
                        } catch (JSONException e) {
                            // Something went wrong!
                        }
                    }

*/
                } catch (final JSONException e) {
                    Log.e(TAG, "Json parsing error: " + e.getMessage());

                }

            } else {
                Log.e(TAG, "Couldn't get json from server.");
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {

                        Snackbar.make(root, "Could not connect with server", Snackbar.LENGTH_LONG)
                                .setAction("Action", null).show();
                    }
                });


            }

            return null;
        }

        @Override
        protected void onPostExecute(Void result) {
            super.onPostExecute(result);
            /*ListAdapter adapter = new SimpleAdapter(MainActivity.this, contactList,
                    R.layout.list_item, new String[]{ "email","mobile"},
                    new int[]{R.id.email, R.id.mobile});
            lv.setAdapter(adapter);*/
            try {

                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                     if(isKeyboardConnected==true){
                         fab_keyboard.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                     }
                     if(isMouseConnected==true){
                         fab_mouse.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                     }
                        showFABMenu();
                    }
                });

            }
            catch(Exception e){
                Log.e(TAG,e.getMessage());
            }
        }
    }
    /**
     * Gets the list of software installed on particular system
     * @see android.os.AsyncTask
     */
    private class GetSoftwares extends AsyncTask<Void, Void, Void> {


        @Override
        protected void onPreExecute() {
            super.onPreExecute();


        }

        @Override
        protected Void doInBackground(Void... arg0) {
            HttpHandler sh = new HttpHandler();
            // Making a request to url and getting response
            String url = "http://10.130.150.223:5000/lab/"+lab+"/"+pc+"/software";
            String jsonStr = sh.makeServiceCall(url,true);

            //Log.e(TAG, "Response from url: " + jsonStr);
            if (jsonStr != null) {
                try {
                    JSONObject jsonObj = new JSONObject(jsonStr);

                    JSONArray jsonArr=jsonObj.getJSONArray("installed");
                    for(int i=0;i<jsonArr.length();i++){
                        software.add(jsonArr.getString(i));
                    }

                } catch (final JSONException e) {
                    Log.e(TAG, "Json parsing error: " + e.getMessage());
              /*  runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(getApplicatioContext(),
                                "Json parsing error: " + e.getMessage(),
                                Toast.LENGTH_LONG).show();
                    }
                });
*/
                }

            } else {
                Log.e(TAG, "Couldn't get json from server.");
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {

                        Snackbar.make(root, "Could not connect with server", Snackbar.LENGTH_LONG)
                                .setAction("Action", null).show();
                    }
                });
            }

            return null;
        }

        @Override
        protected void onPostExecute(Void result) {
            super.onPostExecute(result);
            /*int size=software.size();
            String[] softArr=new String[size];
            for(int i=0;i<size;i++){
                softArr[i]=software.get(i);
            }
*/
            mAdapter = new SoftListAdapter(software);
            recyclerView.setAdapter(mAdapter);






        }
    }

}
